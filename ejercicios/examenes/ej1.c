/* Tareas con procesos ligeros y semáforos. */

/* 
Para la compilación y ejecución de este problema:
gcc -lpthread ej1.c -o ej1
./ej1
*/

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>


#define N 4

sem_t sem1;
sem_t sem2;
sem_t sem3;
sem_t sem4;

void *tarea1() {
    int i = 0;
    while (i<N) {
        sem_wait(&sem1);
        printf("Tarea 1.\n");
        sem_post(&sem2);
        i++;
    }
    pthread_exit(NULL);
}

void *tarea2() {
    int i = 0;
    while (i<N) {
        sem_wait(&sem2);
        printf("Tarea 2.\n");
        sem_post(&sem3);
        i++;
    }
    pthread_exit(NULL);
}

void *tarea3() {
    int i = 0;
    while (i<N) {
        sem_wait(&sem3);
        printf("Tarea 3.\n");
        sem_post(&sem4);
        i++;
    }
    pthread_exit(NULL);
}

void *tarea4() {
    int i = 0;
    while (i<N) {
        sem_wait(&sem4);
        printf("Tarea 4.\n");
        sem_post(&sem1);
        i++;
    }
    pthread_exit(NULL);
}

void main() {
    pthread_t idthread[4];    

    // Crear los semáforos
    sem_init(&sem1, 0, 1);
    sem_init(&sem2, 0, 0);
    sem_init(&sem3, 0, 0);
    sem_init(&sem4, 0, 0);

    // Crear los hilos
    pthread_create(&idthread[0], NULL, tarea1, NULL);
    pthread_create(&idthread[1], NULL, tarea2, NULL);
    pthread_create(&idthread[2], NULL, tarea3, NULL);
    pthread_create(&idthread[3], NULL, tarea4, NULL);

    // Esperar a que terminen
    for (int i=0; i<N; i++) {
        pthread_join(idthread[i], NULL);
    }

    // Liberar recursos
    sem_destroy(&sem1);
    sem_destroy(&sem2);
    sem_destroy(&sem3);
    sem_destroy(&sem4);

    exit(0);
}