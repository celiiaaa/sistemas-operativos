/* SSOO-P3 23/24 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <pthread.h>
#include "queue.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_full = PTHREAD_COND_INITIALIZER;

/* Crea la cola y reserva el tamaño especificado */
queue* queue_init(int size) {
    // Reservar memoria para la cola
    queue *q = (queue *) malloc(size * sizeof(struct element));
    if (q == NULL) {
        perror("Error: Reservar memoria para la cola.\n");
        return NULL;
    }
    // Inicializar los valores de la cola
    q->head = NULL;
    q->tail = NULL;
    q->size = size;
    q->count = 0;

    return q;
}

/* Inserta elementos en la cola */
int queue_put(queue *q, struct element* x) {
    pthread_mutex_lock(&mutex);
    // Comprobar si la cola está llena
    while (queue_full(q) == 1) {
        pthread_cond_wait(&cond_full, &mutex);
    }
    // Insertar el elemento en la cola
    if (queue_empty(q) == 1) {
        q->head = x;
        q->tail = x;
        q->count++;
        x->next = x;
        x->prev = x;
    } else {
        // Elementos auxiliares
        struct element *prev = q->tail;
        struct element *next = q->head;
        prev->next = x;
        next->prev = x;
        x->next = next;
        x->prev = prev;
        q->tail = x;
        q->count++; 
    }
    // Enviar la señal que la cola no está vacía
    pthread_cond_signal(&cond_empty);
    // Desbloquear el mutex
    pthread_mutex_unlock(&mutex);

    return 0;
}

/* Extrae elementos de la cola */
struct element* queue_get(queue *q) {
    pthread_mutex_lock(&mutex);
    // Comprobar si la cola está vacía
    while (queue_empty(q) == 1) {
        pthread_cond_wait(&cond_empty, &mutex);
    }
    // Extraer el elemento de la cola
    struct element *element = malloc(sizeof(struct element));
    if (q->head == q->tail) {
        element = q->tail;
        q->head = NULL;
        q->tail = NULL;
        q->count--;
    } else {
        struct element *new_head;
        element = q->head;
        new_head = element->next;
        q->tail->next = new_head;
        new_head->prev = q->tail;
        q->head = new_head;
        q->count--;
    }
    // Enviar la señal que la cola no está llena
    pthread_cond_signal(&cond_full);
    // Desbloquear el mutex
    pthread_mutex_unlock(&mutex);

    return element;
}

/* Determina si está vacía */
int queue_empty(queue *q) {
    if (q->count == 0) {
        return 1;
    }

    return 0;
}

/* Determina si está llena */
int queue_full(queue *q) {
    if (q->count == q->size) {
        return 1;
    }

    return 0;
}

/* Elimina la cola y libera recursos */
int queue_destroy(queue *q) {
    struct element *current = q->head;
    struct element *next;
    // Liberar memoria de los elementos
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    // Liberar memoria de la cola
    free(q);
    // Destruir el mutex y las variables de condición
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_empty);
    pthread_cond_destroy(&cond_full);

    return 0;
}

/* Imprime la cola */
void print_queue(queue *q) {
    struct element *current = q->head;
    printf("[\n");
    for (int i=0; i < q->count; i++) {
        printf("Elemento %d: %d %d %d\n", i, current->product_id, current->op, current->units);
        current = current->next;
    }
    printf("]\n");
}