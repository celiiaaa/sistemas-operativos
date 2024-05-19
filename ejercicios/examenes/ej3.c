/* 
Para la compilación y ejecución de este problema:
gcc -lpthread ej3.c -o ej3
./ej3
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#define SIZE_ARRAY 100
#define N_ITER 200

pthread_attr_t attr;
pthread_t idthid[2];
float array[SIZE_ARRAY];

// Contadores para la finalización de los hilos
int contfin1 = 0, contfin2 = 0;

// Sincronización y concurrencia
pthread_mutex_t mutex1, mutex2;
pthread_cond_t cond1, cond2;


void rellenarArray() {
    for (int i=0; i<SIZE_ARRAY; i++) {
        array[i] = (float) rand() / RAND_MAX;
    }
}


void *thread1() {
    printf("Hilo 1.\n");
    int i, j;

    // Inicializar las variables necesarias
    int yoinizializo1 = 0, yoinizializo2 = 0;       // concurrencia y sincronización
    float auxiliar[SIZE_ARRAY/2];                     // array auxiliar

    // Copiar los valores del array en el auxiliar
    for (j=0; j<N_ITER; j++) {
        // El primer cálculo: el último, el primero y el segundo
        auxiliar[0] = (array[SIZE_ARRAY-1] + array[0] + array[1] / 3);
        for (i=1; i<SIZE_ARRAY/2; i++) {
            auxiliar[i] = (array[i-1] + array[i] + array[i+1] / 3);
        }

        // Realizar las operaciones de sincronización necesarias
        // Esperar a que el otro hilo termine su cálculo
        pthread_mutex_lock(&mutex1);
        contfin1 ++;
        if (contfin1 == 1) {
            yoinizializo1 = 1;
        }
        // Esperar a que el otro copie sus datos
        while (contfin1 != 2) {
            pthread_cond_wait(&cond1, &mutex1);
        }
        // Copiar el array
        for (i=0; i<SIZE_ARRAY/2; i++) {
            array[i] = auxiliar[i];
        }
        // 
        pthread_cond_signal(&cond1);
        if (yoinizializo1) {
            contfin1 = 0;
            yoinizializo1 = 0;
        }
        pthread_mutex_unlock(&mutex1);

        // 
        pthread_mutex_lock(&mutex2);
        contfin2 ++;
        if (contfin2 == 1) {
            yoinizializo2 = 1;
        }
        // Esperar a que el otro copie los datos
        while(contfin2 != 2) {
            pthread_cond_wait(&cond2, &mutex2);
        }
        pthread_cond_signal(&cond2);
        if (yoinizializo2) {
            contfin2 = 0;
            yoinizializo2 = 0;
        }
        pthread_mutex_unlock(&mutex2);
    }

    pthread_exit(NULL);
}

void *thread2() {
    printf("Hilo 2.\n");
    int i, j;

    // Inicializar las variables necesarias
    int yoinizializo1 = 0, yoinizializo2 = 0;       // concurrencia y sincronización
    float auxiliar[SIZE_ARRAY/2];                     // array auxiliar

    // Copiar los valores del array en el auxiliar
    for (j=0; j<N_ITER; j++) {
        for (i=SIZE_ARRAY/2; i<SIZE_ARRAY-1; i++) {
            auxiliar[i-SIZE_ARRAY/2] = (array[i-1] + array[i] + array[i+1] / 3);
        }
        // El último cálculo: el penúltimo, el último y el primero.
        auxiliar[SIZE_ARRAY/2-1] = (array[SIZE_ARRAY-2] + array[SIZE_ARRAY-1] + array[0]) / 3;

        // Realizar las operaciones de sincronización necesarias
        // Esperar a que el otro hilo termine su cálculo
        pthread_mutex_lock(&mutex1);
        contfin1 ++;
        if (contfin1 == 1) {
            yoinizializo1 = 1;
        }
        // Esperar a que el otro copie sus datos
        while (contfin1 != 2) {
            pthread_cond_wait(&cond1, &mutex1);
        }
        // Copiar el array
        for (i=SIZE_ARRAY/2; i<SIZE_ARRAY; i++) {
            array[i] = auxiliar[i-SIZE_ARRAY/2];
        }
        // 
        pthread_cond_signal(&cond1);
        if (yoinizializo1) {
            contfin1 = 0;
            yoinizializo1 = 0;
        }
        pthread_mutex_unlock(&mutex1);

        // 
        pthread_mutex_lock(&mutex2);
        contfin2 ++;
        if (contfin2 == 1) {
            yoinizializo2 = 1;
        }
        // Esperar a que el otro copie los datos
        while(contfin2 != 2) {
            pthread_cond_wait(&cond2, &mutex2);
        }
        pthread_cond_signal(&cond2);
        if (yoinizializo2) {
            contfin2 = 0;
            yoinizializo2 = 0;
        }
        pthread_mutex_unlock(&mutex2);
    }

    pthread_exit(NULL);
}


void main() {
    int i;
    rellenarArray();

    // Inicializar los mutex, las variables condicion
    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);
    pthread_attr_init(&attr);

    // Crear los hilos
    pthread_create(&idthid[0], &attr, thread1, NULL);
    pthread_create(&idthid[1], &attr, thread2, NULL);

    // Esperar a que terminen de ejecutar
    for (i=0; i<2; i++) {
        pthread_join(idthid[i], NULL);
    }

    // Liberar recursos
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);
    pthread_cond_destroy(&cond1);
    pthread_cond_destroy(&cond2);

    exit(0);
}