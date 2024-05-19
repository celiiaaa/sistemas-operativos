/* Realizar un programa que reciba un nombre de directorio y se muestre por pantalla los
nombre de ficheros y directorios que contiene, su modo, si tienen o no permiso de lectura
para propietario, si son directorios, y para ficheros modificados en los últimos 10 días, 
su fecha de acceso. */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <time.h>

#define MAX 100

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("ERROR: estructura es: %s <dir_name>.", argv[0]);
        return -1;
    }

    int er;
    char name_dir[MAX];
    char name_file[MAX];


}