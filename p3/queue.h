/* SSOO-P3 23/24 */

#ifndef HEADER_FILE
#define HEADER_FILE

/* Element structure */
struct element {
    int product_id;     /* Identificador del producto */
    int op;             /* Tipo de operación */
    int units;          /*Número de unidades implicadas en la operación */

    struct element *next;       /* Puntero al siguiente elemento */
    struct element *prev;       /* Puntero al elemento anterior */
};

/* Queue structure */
typedef struct queue {
    struct element *head;       /* Puntero a la cabeza de la cola */
    struct element *tail;       /* Puntero a la cola de la cola */
    int size;                   /* Tamaño máximo de la cola */
    int count;                  /* Número actual de elementos en la cola */
} queue;

/* Functions */
queue* queue_init (int size);
int queue_destroy (queue *q);
int queue_put (queue *q, struct element* elem);
struct element * queue_get(queue *q);
int queue_empty (queue *q);
int queue_full(queue *q);
void print_queue(queue *q);

#endif
