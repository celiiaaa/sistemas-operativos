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

int index_hist = 0;
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
    // num_commands--;         // He tenido que añadir esto porque daba error...
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

void siginthandler(int param) {
	printf("**** Exiting MSH **** \n");
    free(history);
	//signal(SIGINT, siginthandler);
	exit(0);
}


/* Función auxiliar para escribir mensajes */
void writeMsg(char *msg, int fd) {
    if ((write(fd, msg, strlen(msg))) < 0) {
        perror("Error. Write msg failed.\n");
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

    // Inicializar la variable de entorno
    setenv("Acc", "0", 1);

	while (1) {
		int status = 0;
		int command_counter = 0;
		int in_background = 0;
		signal(SIGINT, siginthandler);

		if (run_history) {
            run_history = 0;
            // Escribir que se está ejecutando un comando
            char msg[100];
            sprintf(msg, "Ejecutando el comando %d\n", index_hist);
            writeMsg(msg, 2);
            // Asignar valores necesarios
            command_counter = history[index_hist].num_commands;
            in_background = history[index_hist].in_background;
            for (int fd=0; fd<3; fd++) {
                strcpy(filev[fd], history[index_hist].filev[fd]);
            }
            // Asignar comandos y sus argumentos
            for (int i_cmd=0; i_cmd<command_counter; i_cmd++) {
                int n_args = history[index_hist].args[i_cmd];
                argvv[i_cmd] = (char **) calloc(n_args+1, sizeof(char *));
                for (int i_arg=0; i_arg<n_args; i_arg++) {
                    argvv[i_cmd][i_arg] = strdup(history[index_hist].argvv[i_cmd][i_arg]);
                }
                argvv[i_cmd][n_args] = NULL;        // Añadir NULL al final
            }
            argvv[history[index_hist].num_commands+1] = NULL;        // Añadir NULL al final

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

            // Mandato interno mycalc.
            if (strcmp(argvv[0][0], "mycalc") == 0) {
                // Mensaje de error
                char *msg_error = "[ERROR] La estructura del comando es mycalc <operando_1> <add/mul/div> <operando_2>\n";
                if (command_counter==1 && in_background==0 && strcmp(filev[0], "0")==0 && strcmp(filev[1], "0")==0 && strcmp(filev[2], "0")==0) {
                    if (argvv[0][1] != NULL && argvv[0][2] && argvv[0][3]) {
                        if (strcmp(argvv[0][1], "cero")!=0 && strcmp(argvv[0][3], "cero")!=0 && ((strcmp(argvv[0][1], "0")!=0 && atoi(argvv[0][1])!=0) || (strcmp(argvv[0][3], "0")!=0 && atoi(argvv[0][3])!=0))) {
                            int op1 = atoi(argvv[0][1]);
                            int op2 = atoi(argvv[0][3]);
                            char result[100];
                            // Suma
                            if (strcmp(argvv[0][2], "add") == 0) {
                                char *str_acc = getenv("Acc");
                                int acc = 0;
                                if (str_acc != NULL) {
                                    acc = atoi(str_acc);
                                }
                                int suma = op1 + op2;
                                acc += suma;
                                char new_acc[10];
                                sprintf(new_acc, "%d", acc);
                                if (setenv("Acc", new_acc, 1) != 0) {
                                    perror("Error. Acc setenv.\n");
                                    exit(-1);
                                }
                                sprintf(result, "[OK] %d + %d = %d; Acc %d\n", op1, op2, suma, acc);
                                writeMsg(result, 2);
                            }
                            // Multiplicación
                            else if (strcmp(argvv[0][2], "mul") == 0) {
                                int multi = op1 * op2;
                                sprintf(result, "[OK] %d * %d = %d\n", op1, op2, multi);
                                writeMsg(result, 2);
                            }
                            // División
                            else if (strcmp(argvv[0][2], "div") == 0) {
                                if (op2 == 0) {
                                    perror("Error. Zero div.\n");
                                } else if (op2 > op1) {
                                    perror("Error. Inf dividend > divisor.\n");
                                } else {
                                    int div = op1 / op2;
                                    int rest = op1 % op2;
                                    sprintf(result, "[OK] %d / %d = %d; Resto %d\n", op1, op2, div, rest);
                                    writeMsg(result, 2);
                                }
                            }
                            // Error
                            else {
                                writeMsg(msg_error, 1);
                            }
                        } else {
                            writeMsg(msg_error, 1);
                        }
                    } else {
                        writeMsg(msg_error, 1);
                    }
                } else {
                    writeMsg(msg_error, 1);
                }
            }

            // Mandato interno myhist.
            else if (strcmp(argvv[0][0], "myhistory") == 0) {
                // Mensaje de error
                char *msg_error = "[ERROR] La estructura del comando es myhistory <N*>\n";
                if (command_counter==1 && in_background==0 && strcmp(filev[0], "0")==0 && strcmp(filev[1], "0")==0 && strcmp(filev[2], "0")==0) {
                    // Imprimir el historial
                    if (argvv[0][1] == NULL) {
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
                    // Ejecutar comando N
                    else if ((strcmp(argvv[0][1], "cero") != 0) || (strcmp(argvv[0][1], "0")!= 0 && atoi(argvv[0][1])!=0)) {
                        int n = atoi(argvv[0][1]);
                        char *msg_err = "ERROR: Comando no encontrado\n";
                        if (n >= 0 || n < 20) {
                            // Buscar comando.
                            index_hist = head;
                            int en = -1;
                            while (index_hist != tail) {
                                if (index_hist == n) {
                                    en = 0;
                                    run_history = 1;
                                    break;
                                }
                                index_hist = (index_hist + 1) % history_size;
                            }
                            // No se ha encontrado
                            if (en == -1) {
                                writeMsg(msg_err, 1);
                            }
                        }
                        // Se encuentra fuera de los límites
                        else {
                            writeMsg(msg_err, 1);
                        }
                    }
                    // Error
                    else {
                        writeMsg(msg_error, 1);
                    }
                } else {
                    writeMsg(msg_error, 1);
                }
            }

            // Comando simple
            else if (command_counter == 1) {

                // Guardar el comando en el historial
                store_command(argvv, filev, in_background, &history[tail]);
                tail = (tail + 1) % history_size;
                if (n_elem < history_size) {
                    n_elem++;
                } else {
                    free_command(&history[head]);
                    head = (head + 1) % history_size;
                }

                // Fork
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
                        // Cerrar la entrada estándar
                        if ((close(0)) < 0) {
                            perror("Error. Close failed.\n");
                            exit(-1);
                        }
                        // Abrir el fichero
                        if ((open(filev[0], O_RDONLY, 0666)) < 0) {
                            perror("Error. Open failed.\n");
                            exit(-1);
                        }
                    }
                    // Fichero de salida
                    if (strcmp(filev[1], "0") != 0) {
                        // Cerrar la salida estándar
                        if ((close(1)) < 0) {
                            perror("Error. Close failed.\n");
                            exit(-1);
                        }
                        // Abrir el fichero
                        if ((open(filev[1], O_CREAT | O_WRONLY | O_TRUNC, 0666)) < 0) {
                            perror("Error. Open failed.\n");
                            exit(-1);
                        }
                    }
                    // Fichero de error
                    if (strcmp(filev[2], "0") != 0) {
                        // Cerrar la salida estándar de error
                        if ((close(2)) < 0) {
                            perror("Error. Close failed.\n");
                            exit(-1);
                        }
                        // Abrir el fichero
                        if ((open(filev[2], O_CREAT | O_WRONLY | O_TRUNC, 0666)) < 0) {
                            perror("Error. Open failed.\n");
                            exit(-1);
                        }
                    }
                    // Obtener el comando
                    getCompleteCommand(argvv, 0);
                    // Ejecucion del comando
                    if (execvp(argv_execvp[0], argv_execvp) < 0) {
                        perror("Error. Execvp failed.\n");
                        exit(-1);
                    }
                    break;
                default:    // Padre.
                    if (in_background == 0) {
                        // Foreground
                        while (wait(&status) > 0) {
                            if (status < 0) {
                                perror("Error. Child execution failed.\n");
                                exit(-1);
                            }
                        }
                    } else {
                        // Background
                        printf("[%d]\n", getpid());
                        // signal(SIGCHLD, SIG_IGN);
                    }
                    break;
                }
            }

            // Secuencia de comandos
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
                // Duplicar la entrada
                if ((fdin = dup(0)) < 0) {
                    perror("Error. Dup failed.\n");
                    exit(-1);
                }
                // Recorrer todos los comandos
                for (int i=0; i<command_counter; i++) {
                    // Crear siguiente pipe, excepto si es el último comando
                    if (i != command_counter-1) {
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
                        // Redireccion salida estandar de error
                        if (strcmp(filev[2], "0") != 0) {
                            // Cerrar la salida estándar de error
                            if (close(2) < 0) {
                                perror("Error. Close failed.\n");
                                exit(-1);
                            }
                            // Abrir el fichero
                            int fd_in;
                            if (fd_in = (open(filev[2], O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
                                perror("Error. Open failed.\n");
                                exit(-1);
                            }
                            // Duplicar el fichero
                            if ((dup(fd_in)) < 0) {
                                perror("Error. Dup failed.\n");
                                exit(-1);
                            }
                        }

                        // Primer comando, redirecciona la entrada estandar
                        if (i == 0 && strcmp(filev[0], "0") != 0) {
                            // Cerrar la entrada estandar
                            if (close(0) < 0) {
                                perror("Error. Close failed.\n");
                                exit(-1);
                            }
                            // Abirr el fichero
                            int fd_out;
                            if (fd_out = (open(filev[0], O_RDWR, 0666)) < 0) {
                                perror("Error. Open failed.\n");
                                exit(-1);
                            }
                            // Duplicar el fichero
                            if (dup(fd_out) < 0) {
                                perror("Error. Dup failed.\n");
                                exit(-1);
                            }
                        }
                        // La entrada del comando actual será la salida del comando anterior
                        else {
                            // Cerrar la entrada estandar
                            if (close(0) < 0) {
                                perror("Error. Close failed.\n");
                                exit(-1);
                            }
                            // Duplicar el descriptor asociado a la entrada
                            if (dup(fdin) < 0) {
                                perror("Error. Dup failed.\n");
                                exit(-1);
                            }
                            // Cerrar el descriptor asociado
                            if (close(fdin) < 0) {
                                perror("Error. Close failed.\n");
                                exit(-1);
                            }
                        }

                        // Último comando, redirecciona la salida estandar
                        if (i == command_counter-1) {
                            if (strcmp(filev[1], "0") == 1) {
                                // Cerrar la salida estandar
                                if (close(1) < 0) {
                                    perror("Error. Close failed.\n");
                                    exit(-1);
                                }
                                // Abrir el fichero
                                int fd_out;
                                if (fd_out = (open(filev[1], O_TRUNC | O_WRONLY | O_CREAT, 0666)) < 0) {
                                    perror("Error. Open failed.\n");
                                    exit(-1);
                                }
                                // Duplicar el fichero
                                if (dup(fd_out) < 0) {
                                    perror("Error. Dup failed.\n");
                                    exit(-1);
                                }
                            }
                        }
                        // Redirecciona la salida estandar el proceso actual a la salida de la tubería
                        else {
                            // Cerrar la salida estandar
                            if (close(1) < 0) {
                                perror("Error. Close failed.\n");
                                exit(-1);
                            }
                            // Duplicar el descriptor de fichero asociado a la salida
                            if (dup(fd[1]) < 0) {
                                perror("Error. Dup failed.\n");
                                exit(-1);
                            }
                            // Cerrar el de entrada
                            if (close(fd[0]) < 0) {
                                perror("Error. Close failed.\n");
                                exit(-1);
                            }
                            // Cerrar el de salida
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
                    // signal(SIGCHLD, SIG_IGN);
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
        // Comprobar que no se produjo error de lectura
        else if (command_counter < 0) {
            perror("Error. Read command failed.\n");
            return -1;
        }
	}
	
    // Fin.
    return 0;
}
