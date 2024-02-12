//P1-SSOO-23/24

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>


int main(int argc, char *argv[]) {
	/* Comprobar que se pasen el número de parámetros correctos */
	if(argc < 3) {
		printf("Error. Too few arguments\n");
		return -1;
	}

	/* Abrir el directorio pasado como parámetro. */
	DIR *dir = opendir(argv[1]);
	if(dir == NULL) {
		printf("Error. Directory not found\n");
		return -1;
	}

	/* Leer todas las entradas */
	struct dirent *entrada;
	int encontrado = 0;
	while((entrada = readdir(dir)) != NULL) {
		/* Comprobar que sea el fichero deseado */
		if(strcmp(entrada->d_name, argv[2]) == 0) {
			printf("File %s is in directory %s\n", argv[2], argv[1]);
			encontrado = 1;
		}
	}

	/* Comprobar que el fichero haya sido encontrado */
	if (!encontrado) {
		printf("File %s is not in directory %s\n", argv[2], argv[1]);
	}

	/* Cerrar el directorio */
	if(closedir(dir) == -1) {
		printf("Error al cerrar el directorio");
		return -1;
	}

	return 0;
}

