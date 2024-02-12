//P1-SSOO-23/24

#include <stdio.h>		// Header file for system call printf
#include <unistd.h>		// Header file for system call gtcwd
#include <sys/types.h>	// Header file for system calls opendir, readdir y closedir
#include <dirent.h>
#include <string.h>

#define PATH_MAX 4096


int main(int argc, char *argv[]) {
	char path_dir[PATH_MAX];
	struct dirent *entrada;
	DIR *dir;

	/* En caso de no especificar ningun directorio */
	if(argc == 1) {
		if(getcwd(path_dir, sizeof(path_dir)) == NULL) {
			printf("Error al obtener el path del directorio actual");
			return -1;
		}

		/* Abrir el directorio */
		dir = opendir(path_dir);
	}

	/* En caso de especificar un directorio */
	else {
		/* Comprobar que se pase un solo argumento */
		if(argc > 2) {
			printf("Error. Too many arguments\n");
			return -1;
		}

		/* Abrir el directorio */
		dir = opendir(argv[1]);
	}

	/* En caso de no encontrar el directorio */
	if(dir == NULL) {
		printf("Error. Directory not found\n");
		return -1;
	}

	/* Leer el contenido del directorio e imprimirlo */
	while((entrada = readdir(dir)) != NULL) {
		/* PREGUNTAR SI PUEDO HACER ESTO */
		if(strcmp(entrada->d_name, ".")!=0 && strcmp(entrada->d_name, "..")!=0) {
			printf("%s\n", entrada->d_name);
		}
	}

	/* Cerrar el directorio */
	if(closedir(dir) == -1) {
		printf("Error al cerrar el directorio");
		return -1;
	}

	return 0;
}
