/* Ejercicio de sustituir un caracter por otro. */

/* 
Para compilar y ejecutar:
gcc -o ej4 ej4.c
./ej4 <file_in> <file_out> <char_old> <char_new>
*/

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>


int sustituir(char *filename_in, char *filename_out, char car_old, char car_new) {
    int contador = 0;
    FILE *fd_in, *fd_out;

    // Abrir el fichero in
    if ((fd_in = fopen(filename_in, "r")) == NULL) {
        perror("ERROR: Apertura del fichero de entrada.\n");
        return -1;
    }

    // Abrir el fichero out
    if ((fd_out = fopen(filename_out, "w")) == NULL) {
        perror("ERROR: Apertura del fichero de salida.\n");
        return -1;
    }

    // Leer el fichero de entrada y escribir en el de salida
    char car_actual;
    while((car_actual = getc(fd_in)) != EOF) {
        // Si se trata del caracter buscado, se modifica con el nuevo valor
        if (car_actual == car_old) {
            car_actual = car_new;
            contador ++;
        }

        // Escribir el caracter en el fichero de salida
        putc(car_actual, fd_out);
    }

    // Cerrar el fichero de entrada
    if (fclose(fd_in) < 0) {
        perror("ERROR: Cierre del fichero de entrada.\n");
        return -1;
    }

    // Cerrar el fichero de salida
    if (fclose(fd_out) < 0) {
        perror("ERROR: Cierre del fichero de salida.\n");
        return -1;
    }

    return contador;
}

void main(int argc, char *argv[]) {
    // Comprobar la entrada
    if (argc != 5) {
        char msg[1024];
        sprintf(msg, "ERROR: El comando es: %s <file_in> <file_out> <char_old> <char_new>.\n", argv[0]);
        perror(msg);
        exit(-1);
    }

    // Comprobar los caracteres
    if (strlen(argv[3]) != 1) {
        char msg[1024];
        sprintf(msg, "ERROR: Has introducido %s y debe ser un solo caracter.\n", argv[3]);
        perror(msg);
        exit(-1);
    } 
    if (strlen(argv[4]) != 1) {
        char msg[1024];
        sprintf(msg, "ERROR: Has introducido %s y debe ser un solo caracter.\n", argv[4]);
        perror(msg);
        exit(-1);
    }

    // Llamar a la función
    int result = sustituir(argv[1], argv[2], argv[3][0], argv[4][0]);

    // Comprobar el resultado
    if (result == -1) {
        exit(-1);
    } else {
        printf("Sustituiciones realizadas = %d.\n", result);
        exit(0);
    }
}