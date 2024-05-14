/* Escriba un programa que use los servicios POSIX de proyección de archivos para comparar
dos archivos. El programa recibe como argumentos los nombre de los archivos a comparar. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        perror("ERROR. El comando es: ./ej1 <file1> <file2>.");
        return -1;
    }

    int fd1, fd2;
    int result;
    struct stat bstat;
    int size1, size2;

    // Abrir el archivo 1
    char *path_file1 = argv[1];
    if ((fd1 = open(path_file1, O_RDONLY)) < 0) {
        perror("ERROR: abrir el fichero 1.");
        return -1;
    }
    // Abrir el archivo 2
    char *path_file2 = argv[2];
    if ((fd2 = open(path_file2, O_RDONLY)) < 0) {
        perror("ERROR: abrir el fichero 2.");
        return -1;
    }

    // Obtener el número de caracteres del archivo 1
    if (fstat(fd1, &bstat) < 0) {
        perror("ERROR: fstat del archivo 1.");
        return -1;
    }
    size1 = bstat.st_size;
    // Obtener el número de caracteres del archivo 2
    if (fstat(fd2, &bstat) < 0) {
        perror("ERROR: fstat del archivo 2.");
        return -1;
    }
    size2 = bstat.st_size;

    // Comparar el tamaño
    if (size1 == size2) {
        
        // Proyectar el archivo 1 en memoria
        char *mem1;
        if ((mem1 = mmap(NULL, size1, PROT_READ, MAP_PRIVATE, fd1, 0)) < 0) {
            perror("ERROR: mmap del archivo 1.");
            return -1;
        }
        // Proyectar el archivo 2 en memoria
        char *mem2;
        if ((mem2 = mmap(NULL, size2, PROT_READ, MAP_PRIVATE, fd2, 0)) < 0) {
            perror("ERROR: mmap del archivo 2.");
            return -1;
        }

        // Comparar las zonas proyectadas en memoria
        if (memcmp(mem1, mem2, size1) == 0) {
            result = 1;
        }

        // Eliminar las proyecciones
        if (munmap(mem1, size1) < 0) {
            perror("ERROR: munmap del archivo 1.");
            return -1;
        }
        if (munmap(mem2, size2) < 0) {
            perror("ERROR: munmap del archivo 1.");
            return -1;
        }
    } 
    
    else {
        result = 0;
    }

    // Cerrar los archivos
    if (close(fd1) < 0) {
        perror("ERROR: cerrar el archivo 1.");
        return -1;
    }
    if (close(fd2) < 0) {
        perror("ERROR: cerrar el archivo 2.");
        return -1;
    }


    if (result == 1) {
        print("El fichero %s y el fichero %s son iguales!", path_file1, path_file2);
    } else {
        print("El fichero %s y el fichero %s no son iguales!", path_file1, path_file2);

    }

    return result;
}
