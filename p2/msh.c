/* P2-SSOO-23/24 */

// #include "parser.h"
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_COMMANDS 8

/* files in case of redirection */ 
char filev[3][64];

/* to store the execvp second parameter */
char *argv_execvp[8];

void siginthandler(int param) {
	printf("**** Exiting MSH **** \n");
	//signal(SIGINT, siginthandler);
	exit(0);
}

/* myhistory */

struct command {
  // Store the number of commands in argvv
  int num_commands;
  // Store the number of arguments of each command
  int *args;
  // Store the commands
  char ***argvv;
  // Store the I/O redirection
  char filev[3][64];
  // Store if the command is executed in background or foreground
  int in_background;
};

int history_size = 20;
struct command *history;
int head = 0;
int tail = 0;
int n_elem = 0;

void free_command(struct command *cmd) {
    if((*cmd).argvv != NULL) {
        char **argv;
        for (; (*cmd).argvv && *(*cmd).argvv; (*cmd).argvv++) {
            for (argv = *(*cmd).argvv; argv && *argv; argv++) {
                if(*argv) {
                    free(*argv);
                    *argv = NULL;
                }
            }
        }
    }
    free((*cmd).args);
}

void store_command(char ***argvv, char filev[3][64], int in_background, struct command* cmd) {
    int num_commands = 0;
    while (argvv[num_commands] != NULL) {
        num_commands++;
    }

    for (int f=0;f < 3; f++) {
        if (strcmp(filev[f], "0") != 0) {
            strcpy((*cmd).filev[f], filev[f]);
        }
        else {
            strcpy((*cmd).filev[f], "0");
        }
    }

    (*cmd).in_background = in_background;
    (*cmd).num_commands = num_commands-1;
    (*cmd).argvv = (char ***) calloc((num_commands) ,sizeof(char **));
    (*cmd).args = (int*) calloc(num_commands , sizeof(int));

    for( int i = 0; i < num_commands; i++) {
        int args= 0;
        while (argvv[i][args] != NULL) {
            args++;
        }
        (*cmd).args[i] = args;
        (*cmd).argvv[i] = (char **) calloc((args+1) ,sizeof(char *));
        int j;
        for (j=0; j<args; j++) {
            (*cmd).argvv[i][j] = (char *)calloc(strlen(argvv[i][j]),sizeof(char));
            strcpy((*cmd).argvv[i][j], argvv[i][j] );
        }
    }
}


/**
 * Get the command with its parameters for execvp
 * Execute this instruction before run an execvp to obtain the complete command
 * @param argvv
 * @param num_command
 * @return
 */
void getCompleteCommand(char*** argvv, int num_command) {
	//reset first
	for(int j = 0; j < 8; j++) {
		argv_execvp[j] = NULL;
    }
	int i = 0;
	for (i = 0; argvv[num_command][i] != NULL; i++) {
		argv_execvp[i] = argvv[num_command][i];
    }
}


void writeMsg(char *msg, int fd) {
    if ((write(fd, msg, strlen(msg))) < 0) {
        perror("Error. Write error_msg failed.\n");
        exit(-1);
    }
}


/**
 * Main sheell  Loop  
 */
int main(int argc, char* argv[]) {
	/**** Do not delete this code.****/
	int end = 0; 
	int executed_cmd_lines = -1;
	char *cmd_line = NULL;
	char *cmd_lines[10];
    setenv("Acc", "0", 1);

	if (!isatty(STDIN_FILENO)) {
		cmd_line = (char*)malloc(100);
		while (scanf(" %[^\n]", cmd_line) != EOF){
			if(strlen(cmd_line) <= 0) return 0;
			cmd_lines[end] = (char*)malloc(strlen(cmd_line)+1);
			strcpy(cmd_lines[end], cmd_line);
			end++;
			fflush (stdin);
			fflush(stdout);
		}
	}

	/*********************************/

	char ***argvv = NULL;
	int num_commands;

	history = (struct command*) malloc(history_size *sizeof(struct command));
	int run_history = 0;

	while (1) {
		int status = 0;
		int command_counter = 0;
		int in_background = 0;
		signal(SIGINT, siginthandler);

		if (run_history) {
            run_history=0;
        } else {
            // Prompt 
            write(STDERR_FILENO, "MSH>>", strlen("MSH>>"));

            // Get command
            //********** DO NOT MODIFY THIS PART. IT DISTINGUISH BETWEEN NORMAL/CORRECTION MODE***************
            executed_cmd_lines++;
            if (end != 0 && executed_cmd_lines < end) {
                command_counter = read_command_correction(&argvv, filev, &in_background, cmd_lines[executed_cmd_lines]);
            } else if (end != 0 && executed_cmd_lines == end) {
                return 0;
            } else {
                command_counter = read_command(&argvv, filev, &in_background); //NORMAL MODE
            }
        }
		//************************************************************************************************


		/************************ STUDENTS CODE ********************************/
	    if (command_counter > 0) {
			if (command_counter > MAX_COMMANDS){
				printf("Error: Maximum number of commands is %d \n", MAX_COMMANDS);
			} else {
                // Obtener todos los comandos
                for (int i=0; i<command_counter; i++) {
                    // Print command
				    print_command(argvv, filev, in_background);
                    // Obtener el comando completo
                    getCompleteCommand(argvv, i);
                    // Almacenar el comando en el historial
                    if (n_elem < history_size) {
                        n_elem++;
                    } else {
                        free_command(&history[head]);
                        head = (head + 1) % history_size;
                    }
                    store_command(argvv, filev, in_background, &history[tail]);
                    tail = (tail + 1) % history_size;
                }
			}

            if (strcmp("mycalc", argv_execvp[0]) == 0) {
                // Comprobar sintaxis
                char *m_error = "[ERROR] La estructura del comando es mycalc <operando_1> <add/mul/div> <operando_2>\n";
                if (strcmp(filev[0], "0")!=0 || strcmp(filev[1], "0")!=0 || strcmp(filev[2], "0")!=0 || in_background!=0 || command_counter!=1) {
                    writeMsg(m_error, 1);
                } else if (argv_execvp[1]==NULL || argv_execvp[2]==NULL || argv_execvp[3]==NULL) {
                    writeMsg(m_error, 1);
                } else if (strcmp("add", argv_execvp[2])!=0 && strcmp("mul", argv_execvp[2])!=0 && strcmp("div", argv_execvp[2])!=0) {
                    writeMsg(m_error, 1);
                } else if ((strcmp(argv_execvp[1], "0")!=0 && atoi(argv_execvp[1])==0) || (strcmp(argv_execvp[3], "0")!=0 && atoi(argv_execvp[3])==0)) {
                    writeMsg(m_error, 1);
                } else {
                    // Operar
                    char result[100];
                    int op1 = atoi(argv_execvp[1]);
                    int op2 = atoi(argv_execvp[3]);
                    if (strcmp("add", argv_execvp[2])==0) {
                        // Suma y acc
                        char *str_acc = getenv("Acc");
                        int acc = atoi(str_acc);
                        int suma = op1 + op2;
                        acc = acc + suma;
                        char new_acc[10];
                        sprintf(new_acc, "%d", acc);
                        setenv("Acc", new_acc, 1);
                        sprintf(result, "[OK] %d + %d = %d; Acc %s\n", op1, op2, suma, getenv("Acc"));
                    } else if (strcmp("mul", argv_execvp[2])==0) {
                        // Multiplicación
                        int multi = op1 * op2;
                        sprintf(result, "[OK] %d * %d = %d\n", op1, op2, multi);
                    } else if (strcmp("div", argv_execvp[2])==0) {
                        // División
                        // REVISAR
                        if (op2 == 0) {
                            perror("Error. Zero div.\n");
                        } else if (op2 > op1) {
                            perror("Error. Inf dividend > divisor.\n");
                        } else {
                            int div = op1 / op2;
                            int rest = op1 % op2;
                            sprintf(result, "[OK] %d / %d = %d; Resto %d\n", op1, op2, div, rest);
                        }
                    }
                    writeMsg(result, 2);
                }

            } else if (strcmp("myhist", argv_execvp[0]) == 0) {
                // Comprobar sintaxis
                char *m_error = "[ERROR] La estructura del comando es myhist <N*>\n";
                if (strcmp(filev[0], "0")!=0 || strcmp(filev[1], "0")!=0 || strcmp(filev[2], "0")!=0 || in_background!=0 || command_counter!=1) {
                    writeMsg(m_error, 1);
                } else if (argv_execvp[1] != NULL && ((strcmp(argv_execvp[1], "0")!=0 && atoi(argv_execvp[1])==0) || (atoi(argv_execvp[1])<0) || (atoi(argv_execvp[1])>20))) {
                    writeMsg(m_error, 1);
                } else {
                    // Ejecutar el comando
                    if (argv_execvp[1] == NULL) {
                        // print_20_comandos_hist
                        int i = head;
                        while (i != tail) {
                            printf("%d %s\n", i, *history[i].argvv[0]);
                            i = (i + 1) % history_size;
                        }
                    } else {
                        printf("Exec\n");
                        // exec N_command
                    }
                }
                
            } /*else if (command_counter == 1) {
                // Mandato simple
                int status;
                pid_t pid = fork();
                switch(pid) {
                    case -1:    // Error
                        perror("Error. Fork failed.\n");
                        exit(-1);
                    case 0:     // Hijo
                        
                        // Fichero de entrada
                        if (strcmp(filev[0], "0") != 0) {
                            if ((close(0)) < 0) {
                                perror("Error. Close failed.\n");
                                exit(-1);
                            }
                            if ((open(filev[0], O_RDWR, 0644)) < 0) {
                                perror("Error. Open failed.\n");
                                exit(-1);
                            }
                        }
                        // Fichero de salida
                        if (strcmp(filev[1], "0") != 0) {
                            if ((close(1)) < 0) {
                                perror("Error. Close failed.\n");
                                exit(-1);
                            }
                            if ((open(filev[1], O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0) {
                                perror("Error. Open failed.\n");
                                exit(-1);
                            }
                        }
                        // Fichero de error
                        if (strcmp(filev[2], "0") != 0) {
                            if ((close(2)) < 0) {
                                perror("Error. Close failed.\n");
                                exit(-1);
                            }
                            if ((open(filev[2], O_CREAT | O_WRONLY | O_TRUNC, 0644)) < 0) {
                                perror("Error. Open failed.\n");
                                exit(-1);
                            }
                        }

                        // Ejecucion del comando
                        if (execvp(argv_execvp[0], argv_execvp) < 0) {
                            perror("Error. Execvp failed.\n");
                            exit(-1);
                        }

                        printf("Fin proceso hijo.\n");
                        break;
                    default:    // Padre.
                        // ...
                        if (wait(&status) == -1) {
                            perror("Error. Wait failed.\n");
                            exit(-1);
                        }
                        break;
                }
            } else {
                // Mandato compuesto
                int status2;
                pid_t pid2;
                int fd[2];
                int fdin;

                // Duplicar el fichero de entrada
                if ((fdin = dup(0)) < 0) {
                    perror("Error. Dup failed.\n");
                    exit(-1);
                }

                // Ejecutar cada uno de los mandatos
                for (int i=0; i<command_counter; i++) {
                    // Crear pipe
                    if (i < command_counter - 1) {
                        if (pipe(fd) < 0) {
                            perror("Error. Pipe failed.\n");
                            exit(-1);
                        }
                    }

                    // Ejecutar el mandato
                    pid2 = fork();
                    switch(pid2) {
                        case -1:    // Error
                            perror("Error. Fork failed.\n");
                            exit(-1);
                        case 0:     // Hijo
                            // Redirecciones
                            
                    }
                }
            }*/

		} else if (command_counter < 0) {
            perror("Error. Read command failed.\n");
            return -1;
        }
	}
	
	return 0;
}
