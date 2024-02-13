//P1-SSOO-23/24

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>


int main(int argc, char *argv[]) {
	/* Comprobar que se pasen el número de parámetros correctos */
	if(argc < 2) {
		printf("Error. Too few arguments\n");
		return -1;
	}

	/* Abrir el archivo pasado como parámetro. */
	int fd = open(argv[1], O_RDONLY);
	if(fd == -1) {
		printf("Error. File not found\n");
		return -1;
	}

	/* Leer el contenido del fichero de byte en byte */
	char caracter;
	int n_bytes = 0;
	int n_palabras = 0;
	int n_lineas = 0;
	int es_palabra = 0;
	while(read(fd, &caracter, 1) > 0) {
		n_bytes++;

		/* En caso de ser salto de linea */
		if(caracter == '\n') {
			n_lineas++;
			if(es_palabra) {
				n_palabras++;
				es_palabra = 0;
			}
		}

		/* En caso de ser un espacio o una tabulacion */
		else if(caracter == ' ' || caracter == '\t') {
			if(es_palabra) {
				n_palabras++;
				es_palabra = 0;
			}
		}

		/* En otro caso, es caracter de una palabra */
		else {
			es_palabra = 1;
		}

	}

	/* Es la última palabra */
	if (es_palabra) {
		n_palabras++;
	}

	/* Imprimir el resultado obtenido */
	printf("%d %d %d %s\n", n_lineas, n_palabras, n_bytes, argv[1]);

	/* Cerrar el archivo */
	if(close(fd) == -1) {
		printf("Error. File not closed\n");
		return -1;
	}

	return 0;
}
