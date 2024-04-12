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

/* background processes -> zombies or not finished */
int *background_processes;
int background_processes_counter = 0;

/* to store the pids */
int *pids;

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
    (*cmd).argvv = (char ***) calloc((num_commands), sizeof(char **));
    (*cmd).args = (int*) calloc(num_commands, sizeof(int));

    for( int i = 0; i < num_commands; i++) {
        int args= 0;
        while (argvv[i][args] != NULL) {
            args++;
        }
        (*cmd).args[i] = args;
        (*cmd).argvv[i] = (char **) calloc((args+1), sizeof(char *));
        int j;
        for (j=0; j<args; j++) {
            (*cmd).argvv[i][j] = (char *) calloc(strlen(argvv[i][j]), sizeof(char));
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


/* Función auxiliar para escribir mensajes */
void writeMsg(char *msg, int fd) {
    if ((write(fd, msg, strlen(msg))) < 0) {
        perror("Error. Write msg failed.\n");
        exit(-1);
    }
}

/* Mycalc */
void suma(int op1, int op2) {
    char result[100];
    char *str_acc = getenv("Acc");
    int acc = atoi(str_acc);
    int suma = op1 + op2;
    acc = acc + suma;
    char new_acc[10];
    sprintf(new_acc, "%d", acc);
    setenv("Acc", new_acc, 1);
    sprintf(result, "[OK] %d + %d = %d; Acc %s\n", op1, op2, suma, getenv("Acc"));
    writeMsg(result, 2);
}

void multi(int op1, int op2) {
    char result[100];
    int multi = op1 * op2;
    sprintf(result, "[OK] %d * %d = %d\n", op1, op2, multi);
    writeMsg(result, 2);
}

void divi(int op1, int op2) {
    if (op2 == 0) {
        perror("Error. Zero div.\n");
    } else if (op2 > op1) {
        perror("Error. Inf dividend > divisor.\n");
    } else {
        char result[100];
        int div = op1 / op2;
        int rest = op1 % op2;
        sprintf(result, "[OK] %d / %d = %d; Resto %d\n", op1, op2, div, rest);
        writeMsg(result, 2);
    }
}

void mycalc(char **argv_execvp) {
    int op1 = atoi(argv_execvp[1]);
    int op2 = atoi(argv_execvp[3]);
    if (strcmp("add", argv_execvp[2])==0) {
        // Suma y acc
        suma(op1, op2);
    } else if (strcmp("mul", argv_execvp[2])==0) {
        // Multiplicación
        multi(op1, op2);
    } else if (strcmp("div", argv_execvp[2])==0) {
        // División
        divi(op1, op2);
    }
}

/* Myhistory */
// Imprimir el historial.
void imprimir_hist() {
    int i = head;
    while (i != tail) {
        char msg[1024];
        int offset = 0;
        if (history[i].argvv[0] != NULL) {
            offset += sprintf(msg+offset, "%d ", i);
            for (int j=0; j<history[i].num_commands; j++) {
                for (int k=0; k<history[i].args[j]; k++) {
                    offset += sprintf(msg+offset, "%s ", history[i].argvv[j][k]);
                }
                if (j<history[i].num_commands-1) {
                    offset += sprintf(msg+offset, " | ");
                }
            }
            if (strcmp(history[i].filev[0], "0") != 0) {
                offset += sprintf(msg+offset, "< %s ", history[i].filev[0]);
            }
            if (strcmp(history[i].filev[1], "0") != 0) {
                offset += sprintf(msg+offset, "> %s ", history[i].filev[1]);
            }
            if (history[i].in_background) {
                offset += sprintf(msg+offset, "&");
            }
            offset += sprintf(msg+offset, "\n");
        }
        writeMsg(msg, 2);
        i = (i + 1) % history_size;
    }
}
// Ejecutar el comando.
void ejecutar_hist(/*struct command cmd, char **argss*/) {
    printf("Ejecutar el comando.\n");
    /*pid_t pid = fork();
    switch (pid) {
        case -1: 
            perror("Error. Fork failed.\n");
            exit(-1);
            break;
        case 0:
            if (execvp(argss[0], argss) < 0) {
                perror("Error. Ececvp failed.\n");
                exit(-1);
                break;
            }
        default:
            int status;
            waitpid(pid, &status, 0);

            break;
    }
    // Liberar la memoria.
    for (int i=0; i<cmd.args[0]; i ++) {
        free(argss[i]);
    }
    free(argss);*/
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

    // setenv("Acc", "0", 1);      // Inicializar la variable de entorno a 0

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
                exit(-1);
			} 
            
            for (int a=0; a<command_counter; a++) {
                getCompleteCommand(argvv, a);
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
                    mycalc(argv_execvp);
                }

            } else if (strcmp("myhist", argv_execvp[0]) == 0) {
                // Comprobar sintaxis
                char *m_error = "[ERROR] La estructura del comando es myhist <N*>\n";
                if (strcmp(filev[0], "0")!=0 || strcmp(filev[1], "0")!=0 || strcmp(filev[2], "0")!=0 || in_background!=0 || command_counter!=1) {
                    writeMsg(m_error, 1);
                } else if (argv_execvp[1] != NULL && ((strcmp(argv_execvp[1], "0")!=0 && atoi(argv_execvp[1])==0) || (atoi(argv_execvp[1])<0) || (atoi(argv_execvp[1])>20))) {
                    writeMsg(m_error, 1);
                } else {
                    // Imprimir 20 ultimos comandos
                    if (argv_execvp[1] == NULL) {
                        imprimir_hist();
                    } 
                    // Ejecutar el comando N
                    else {
                        int n = atoi(argv_execvp[1]);         
                        char *msg_error = "[ERROR] Comando no encontrado\n";        
                        if (n < 0 || n >= 20) {
                            writeMsg(msg_error, 1);
                        } else {
                            char msg[256];
                            // Comprobar que exista ese comando
                            int i = head;
                            int en = -1;
                            while (i != tail) {
                                if (i == n) {
                                    en = 0;
                                    run_history = 1;
                                    sprintf(msg, "Ejecutando el comando %d\n", n);
                                    writeMsg(msg, 2);
                                    // Obtener el comando
                                    struct command cmd = history[i];
                                    char **argss = (char **) malloc(sizeof(char *) * (cmd.args[0] + 1));
                                    for (int i=0; i<cmd.args[0]; i++) {
                                        argss[i] = strdup(cmd.argvv[0][i]);
                                    }
                                    argss[cmd.args[0]] = NULL;         // null final
                                    // Ejecutar.
                                    printf("Ejecutar\n");
                                    /* ejecutar_hist(cmd, argss); */
                                    break;
                                }
                                i = (i + 1) % history_size;
                            }
                            if (en == -1) {
                                writeMsg(msg_error, 1);
                            }
                        }
                    }
                }
            }

            // Mandato externo.
            /* else {
                pids = (int *) malloc(command_counter * sizeof(int));
                int pipefd[command_counter-1][2];

                for (int i=0; i<command_counter; i++) {
                    // Crear el pipe
                    if (pipe(pipefd[i]) < 0) {
                        perror("Error. Create pipe failed.\n");
                        exit(-1);
                    }
                    int pid = fork();

                    switch (pid) {
                        case -1:
                            perror("Error. Fork failed.\n");
                            exit(-1);
                        case 0:
                            // Primer comando
                            if (i == 0) {
                                // Redireccionar la entrada
                                if (strcmp(filev[0], "0") != 0) {
                                    int fdin;
                                    if ((fdin = open(filev[0], O_RDONLY)) < 0) {
                                        perror("Error. Open failed.\n");
                                        exit(-1);
                                    }
                                    dup2(fdin, STDIN_FILENO);
                                    close(fdin);
                                }
                            }
                            // Comando intermedio o final
                            else {
                                dup2(pipefd[i-1][0], STDIN_FILENO);
                                close(pipefd[i-1][1]);
                                waitpid(pids[i-1], &status, 0);
                            }
                            // Cerrar la lectura del pipe actual
                            close(pipefd[i][0]);
                            // Comando final
                            if (i == command_counter-1) {
                                close(pipefd[i][0]);
                                close(pipefd[i][1]);
                                // Redireccionar la salida
                                if (strcmp(filev[1], "0") != 0) {
                                    int fdout;
                                    if ((fdout = open(filev[1], O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
                                        perror("Error. Open failed.\n");
                                        exit(-1);
                                    }
                                    dup2(fdout, STDOUT_FILENO);
                                    close(fdout);
                                }
                                // Redireccionar la salida de error
                                if (strcmp(filev[3], "0") != 0) {
                                    int fderr;
                                    if ((fderr = open(filev[1], O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
                                        perror("Error. Open failed.\n");
                                        exit(-1);
                                    }
                                    dup2(fderr, STDERR_FILENO);
                                    close(fderr);
                                }
                            }

                            // Ejecutar el comando
                            if (execvp(argv_execvp[0], argv_execvp) < 0) {
                                perror("Error. Execvp failed.\n");
                                exit(-1);
                            }

                            break;

                        default:
                            // Añadir el pid al array
                            pids[i] = pid;

                            // Cerrar los pipes que no se usan
                            if (i > 0) {
                                close(pipefd[i-1][0]);
                                close(pipefd[i-1][1]);
                            }

                            // último comando
                            if (i == command_counter-1) {
                                // Cerrar los pipes
                                close(pipefd[i][0]);
                                close(pipefd[i][1]);
                                // Comprobar si es en background o no
                                if (in_background) {
                                    //
                                    printf("[%d]\n", getpid());
                                    background_processes = (int *) realloc(background_processes, (background_processes_counter + command_counter) * sizeof(int));
                                    for (int j=0; j<command_counter; j++) {
                                        background_processes[background_processes_counter-j] = pids[j];
                                    }
                                } else {
                                    // Esperar a que terminen todos los procesos de la secuencia actual
                                    for (int j=command_counter-1; j>=0; j--) {
                                        waitpid(pids[j], &status, 0);
                                    }
                                    // Esperar a que terminen los procesos en background de las secuencias anteriores
                                    int status2;
                                    while (background_processes_counter != 0) {
                                        waitpid(background_processes[background_processes_counter-1], &status2, 0);
                                        background_processes_counter--;
                                    }
                                }
                            }
                            break;
                    }   
                }
            }*/

            // Mandato simple
            else if (command_counter == 1) {
                // Guardo el comando en el historial
                if (n_elem < history_size) {
                    n_elem++;
                } else {
                    free_command(&history[head]);
                    head = (head + 1) % history_size;
                }
                store_command(argvv, filev, in_background, &history[tail]);
                tail = (tail + 1) % history_size;

                int status;
                pid_t pid = fork();
                switch(pid) {
                    case -1:    // Error
                        perror("Error. Fork failed.\n");
                        exit(-1);
                        break;
                    case 0:     // Hijo
                        // Fichero de entrada
                        if (strcmp(filev[0], "0") != 0) {
                            if ((close(0)) < 0) {
                                perror("Error. Close failed.\n");
                                exit(-1);
                            }
                            if ((open(filev[0], O_RDWR, 0666)) < 0) {
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
                            if ((open(filev[1], O_CREAT | O_WRONLY | O_TRUNC, 0666)) < 0) {
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
                            if ((open(filev[2], O_CREAT | O_WRONLY | O_TRUNC, 0666)) < 0) {
                                perror("Error. Open failed.\n");
                                exit(-1);
                            }
                        }

                        // Ejecucion del comando
                        if (execvp(argv_execvp[0], argv_execvp) < 0) {
                            perror("Error. Execvp failed.\n");
                            exit(-1);
                        }

                        break;
                    default:    // Padre.
                        if (in_background != 0) {
                            printf("[%d]\n", getpid());
                            signal(SIGCHLD, SIG_IGN);
                            // usleep(1000);
                        } else {
                            // WEXITSTATUS(status)
                            while (wait(&status) > 0) {
                                if (status < 0) {
                                    perror("Error. Child execution failed.\n");
                                    exit(-1);
                                }
                            }
                        }
                        break;
                } 
            }

            // Mandato compuesto
            else {
                // Guardo el comando en el historial
                if (n_elem < history_size) {
                    n_elem++;
                } else {
                    free_command(&history[head]);
                    head = (head + 1) % history_size;
                }
                store_command(argvv, filev, in_background, &history[tail]);
                tail = (tail + 1) % history_size;

                int status2;
                pid_t pid2;
                int fd[2];
                int fdin;

                // Duplicar el fichero de entrada
                if ((fdin = dup(0)) < 0) {
                    perror("Error. Dup failed.\n");
                    exit(-1);
                }

                // Recorrer todos los comandos
                for (int i=0; i<command_counter; i++) {
                    // Crear siguiente pipe, excepto si es el último comando
                    if (i < command_counter - 1) {
                        if (pipe(fd) < 0) {
                            perror("Error. Create pipe failed.\n");
                            exit(-1);
                        }
                    }

                    // Ejecutar el mandato
                    pid2 = fork();
                    switch(pid2) {
                        case -1:    // Error
                            perror("Error. Fork failed.\n");
                            exit(-1);
                            break;
                        case 0:     // Hijo
                            // Redireccion salida de error
                            if (strcmp(filev[2], "0") != 0) {
                                if (close(2) < 0) {
                                    perror("Error. Close failed.\n");
                                    exit(-1);
                                }
                                if ((open(filev[2], O_TRUNC | O_WRONLY | O_CREAT, 0666)) < 0) {
                                    perror("Error. Open failed.\n");
                                    exit(-1);
                                }
                            }

                            // Primer comando, redirecciona la entrada estandar
                            if (i == 0 && strcmp(filev[0], "0") != 0) {
                                if (close(2) < 0) {
                                    perror("Error. Close failed.\n");
                                    exit(-1);
                                }
                                if ((open(filev[0], O_RDWR, 0666)) < 0) {
                                    perror("Error. Open failed.\n");
                                    exit(-1);
                                }
                            } 
                            // La entrada del comando actual será la salida del comando anterior
                            else {
                                if (close(0) < 0) {
                                    perror("Error. Close failed.\n");
                                    exit(-1);
                                }
                                if (dup(fdin) < 0) {
                                    perror("Error. Dup failed.\n");
                                    exit(-1);
                                }
                                if (close(fdin) < 0) {
                                    perror("Error. Close failed.\n");
                                    exit(-1);
                                }
                            }

                            // Último comando, redirecciona la salida estandar
                            if (i == command_counter-1) {
                                if (strcmp(filev[1], "0") == 1) {
                                    if (close(1) < 0) {
                                        perror("Error. Close failed.\n");
                                        exit(-1);
                                    }
                                    if (open(filev[1], O_TRUNC | O_WRONLY | O_CREAT, 0666) < 0) {
                                        perror("Error. Open failed.\n");
                                        exit(-1);
                                    }
                                }
                            }
                            // Redirecciona la salida estandar el proceso actual a la salida de la tubería
                            else {
                                if (close(1) < 0) {
                                    perror("Error. Close failed.\n");
                                    exit(-1);
                                }
                                if (dup(fd[1]) < 0) {
                                    perror("Error. Dup failed.\n");
                                    exit(-1);
                                }
                                if (close(fd[0]) < 0) {
                                    perror("Error. Close failed.\n");
                                    exit(-1);
                                }
                                if (close(fd[1]) < 0) {
                                    perror("Error. Close failed.\n");
                                    exit(-1);
                                }
                            }

                            // Obtener el comando completo.
                            getCompleteCommand(argvv, i);

                            // Ejecutar el comando actual.
                            if (execvp(argv_execvp[0], argv_execvp) < 0) {
                                perror("Error. Execvp failed.\n");
                                exit(-1);
                            }
                            break;

                        default:    // Padre.
                            // Cerrar fichero fdin
                            if (close(fdin) < 0) {
                                perror("Error. Close failed.\n");
                                exit(-1);
                            }
                            
                            // No último comando
                            if (i != command_counter-1) {
                                // Guardar en fdin la lectura de la tubería para la siguiente iteracion
                                if ((fdin = dup(fd[0])) < 0) {
                                    perror("Error. Dup failed.\n");
                                    exit(-1);
                                }
                                if (close(fd[0]) < 0) {
                                    perror("Error. Close failed.\n");
                                    exit(-1);
                                }
                                if (close(fd[1]) < 0) {
                                    perror("Error. Close failed.\n");
                                    exit(-1);
                                }
                            }
                            break;

                    }
                }

                // El último proceso padre
                if (in_background != 0) {
                    printf("[%d]\n", getpid());
                    signal(SIGCHLD, SIG_IGN);
                    // usleep(100000);
                } else {
                    // WEXITSTATUS(status)
                    while (wait(&status2) < 0) {
                        if (status2 < 0) {
                            perror("Error. Child execution failed.\n");
                            exit(-1);
                        }
                    }
                }
            } 

		} 
        // Comprobar que la lectura no produjo error
        else if (command_counter < 0) {
            perror("Error. Read command failed.\n");
            return -1;
        }

	}
	
    // Fin.
    exit(0);
}
