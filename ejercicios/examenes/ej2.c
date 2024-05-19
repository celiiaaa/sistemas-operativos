/* La criba de Eratostenes. */

/* 
Para la compilación y ejecución de este problema:
gcc -o ej2 ej2.c
./ej2
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>


void main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("ERROR: La estructura del comando es %s <N>.\n", argv[0]);
        exit(-1);
    }

    int n = atoi(argv[1]);
    if (n<=0) {
        printf("ERROR: El valor de N no es válido.\n");
        exit(-1);
    }

    int p[n][2];        // Almacenar los descritores de las pipes.

    // Crear las pipes
    int i;
    for (i=0; i<n; i++) {
        if (pipe(p[i]) < 0) {
            perror("ERROR: Pipe.\n");
            exit(-1);
        }
    }

    // Crear los procesos hijos
    for (i=0; i<n; i++) {
        // verificar que el proceso actual es el hijo.
        if (!fork()) {
            int primo_local, numero;
            // cerrar las pipes
            close(p[i][1]);         // descriptor de escritura del actual pipe
            close(p[i+1][0]);       // descriptor de lectura del siguiente pipe

            // El primer número leido por cada hijo es considerado el número primo local
            read(p[i][0], &primo_local, 4);
            printf("Hijo %d --> numero primo local: %d.\n", getpid(), primo_local);

            while(1) {
                read(p[i][0], &numero, 4);
                // Verficar si es multiplo del primo local y si no es el último
                if ((numero % primo_local) && (i < n - 1)) {
                    // Escribir el numero en el siguiente pipe
                    write(p[i+1][1], &numero, 4);
                }
                // Verificar el final, si el numero es -1
                if (numero == -1) {
                    // Fin
                    break;
                }
            }
            exit(0);
        }
    }

    // Enviar la secuencia de números al hijo 0
    for (i=2; i<n*10; i++) {
        write(p[0][1], &i, 4);
    }
    i = -1;
    write(p[0][1], &i, 4);

    // Esperar a qye terminene todos sus hijos.
    for (i=0; i<n; i++) {
        int status, pid;
        pid = wait(&status);
        printf("Hijo %d finalizó.\n", pid);
    }
}