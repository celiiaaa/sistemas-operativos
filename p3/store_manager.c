/* SSOO-P3 23/24 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "queue.h"

/* Coste y precio de cada producto */
int COSTE[5] = {2, 5, 15, 25, 100};
int PRECIO[5] = {3, 10, 20, 40, 125};

/* Mutex y variables condición */
pthread_mutex_t mutex_prod;
pthread_mutex_t mutex_cons;
pthread_cond_t cond_prod;
pthread_cond_t cond_cons;

/* Orden de las operaciones */
int next_op_prod = 0;
int next_op_cons = 0;

/* Argumentos del hilo productor */
typedef struct arg_productor {
    struct queue *buffer;
    int *ids;
    int max_op;
    struct element *list_operations;
} arg_productor;

/* Función del hilo productor */
void *productor(void *arg) {
    pthread_mutex_lock(&mutex_prod);
    // Obtener los argumentos
    arg_productor *args = (arg_productor *) arg;
    queue *cola_circular = args->buffer;
    int max_op = args->max_op;
    int *ids = args->ids;
    struct element *lista_operaciones = args->list_operations;

    // Realizar las operaciones
    for (int i = 0; i < max_op; i++) {
        
        while (ids[i] != next_op_prod) {
            pthread_cond_wait(&cond_prod, &mutex_prod);
        }

        struct element *operacion = malloc(sizeof(struct element));
        if (operacion == NULL) {
            perror("Error. No se pudo reservar memoria para la operación.\n");
            return NULL;
        }
        *operacion = lista_operaciones[ids[i]];
        
        // Insertar la operación en la cola
        queue_put(cola_circular, operacion);

        // Imprimir la cola. Descomentar para visualizar la cola en cada instante.
        // printf("Cola: ");
        // print_queue(cola_circular);

        next_op_prod++;
        pthread_cond_broadcast(&cond_prod);
    }

    pthread_mutex_unlock(&mutex_prod);

    // Liberar memoria
    free(ids);
    free(args);

    pthread_exit(NULL);
}

/* Argumentos del hilo consumidor */
typedef struct arg_consumidor {
    struct queue *buffer;
    int max_op;
    int *ids;
    int *profit;
    int *product_stock;
} arg_consumidor;

/* Función del hilo consumidor */
void *consumidor(void *arg) {
    pthread_mutex_lock(&mutex_cons);

    // Obtener los argumentos
    arg_consumidor *args = (arg_consumidor *) arg;
    queue *cola_circular = args->buffer;
    int max_op = args->max_op;
    int *ids = args->ids;
    int *profit = args->profit;
    int *product_stock = args->product_stock;

    // Realizar las operaciones
    for (int i = 0; i < max_op; i++) {
        
        while (ids[i] != next_op_cons) {
            pthread_cond_wait(&cond_cons, &mutex_cons);
        }

        // Obtener la operación de la cola
        struct element *operacion = queue_get(cola_circular);
        if (operacion == NULL) {
            perror("Error. No se pudo obtener la operación de la cola.\n");
            return NULL;
        }

        // Realizar la operación
        if (operacion->op == 0) {
            // En caso de compra
            *profit -= COSTE[operacion->product_id-1] * operacion->units;
            product_stock[operacion->product_id-1] += operacion->units;
        } else {
            // En caso de venta
            if (product_stock[operacion->product_id-1] >= operacion->units) {
                *profit += PRECIO[operacion->product_id-1] * operacion->units;
                product_stock[operacion->product_id-1] -= operacion->units;
            } else {
                perror("Error. No hay suficiente stock para realizar la venta.\n");
                pthread_exit(NULL);
            }
        }

        // Imprimir la cola. Descomentar para visualizar la cola en cada instante.
        // printf("Cola: ");
        // print_queue(cola_circular);

        next_op_cons++;
        pthread_cond_broadcast(&cond_cons);
    }

    pthread_mutex_unlock(&mutex_cons);

    // Liberar memoria
    free(ids);
    free(args);

    pthread_exit(NULL);
}

/* Programa principal */
int main (int argc, const char *argv[]) {
    int profit = 0;
    int product_stock[5] = {0, 0, 0, 0, 0};

    // Inicializar los mutex y las variables condición necesarias
    pthread_mutex_init(&mutex_prod, NULL);
    pthread_mutex_init(&mutex_cons, NULL);
    pthread_cond_init(&cond_prod, NULL);
    pthread_cond_init(&cond_cons, NULL);

    // Comprobar el número argumentos
    if (argc != 5) {
        perror("Error. La operacion es ./store_manager <file> <n_prods> <n_cons> <b_size>.\n");
        return -1;
    }

    // Obtener los argumentos y comprobar que son válidos

    // Fichero de entrada
    FILE *fd;
    // Comprobar que se puede abrir el fichero
    if ((fd = fopen(argv[1], "r")) == NULL) {
        perror("Error. No se pudo abrir el fichero.\n");
        return -1;
    }

    // Número de productores
    int n_prod = atoi(argv[2]);
    // Comprobar que sea un número válido
    if (n_prod < 1) {
        perror("Error. El número de productores no es válido.\n");
        return -1;
    }

    // Número de consumidores
    int n_cons = atoi(argv[3]);
    // Comprobar que sea un número válido
    if (n_cons < 1) {
        perror("Error. El número de consumidores no es válido.\n");
        return -1;
    }

    // Tamaño del buffer
    int b_size = atoi(argv[4]);
    // Comprobar que sea un número válido
    if (b_size < 1) {
        perror("Error. El tamaño del buffer no es válido.\n");
        return -1;
    }

    // Leer y cargar los datos del fichero

    // Leer el número de operaciones
    int max_op_file;
    if (fscanf(fd, "%d", &max_op_file) != 1) {
        perror("Error. No se pudo leer el número máximo de operaciones.\n");
        return -1;
    }
    // Reservar memoria para todas las operaciones
    struct element *lista_operaciones;
    lista_operaciones = (struct element *) malloc(max_op_file * sizeof(struct element));
    if (lista_operaciones == NULL) {
        perror("Error. Reserva de memoria para la lista de operaciones.\n");
        return -1;
    }
    // Almacenar todas las operaciones en la lista
    int n = 0;                  // número de operaciones leídas
    while (n < max_op_file) {
        // Leer la linea del fichero
        int id, uds;
        char str_op[10];
        // Comprobar el formato de la linea y almacenarla en las variables
        if (fscanf(fd, "%d %s %d", &id, str_op, &uds) != 3) {
            perror("Error. Línea de operación no válida.\n");
            return -1;
        }

        // Almacenar la operación en la lista
        lista_operaciones[n].product_id = id;
        if (strcmp(str_op, "PURCHASE") == 0) {
            // op = 0 si se trata de la operación compra
            lista_operaciones[n].op = 0;
        } else if (strcmp(str_op, "SALE") == 0) {
            // op = 1 si se trata de la operación venta
            lista_operaciones[n].op = 1;
        } else {
            perror("Error. Operación no válida.\n");
            return -1;
        }
        lista_operaciones[n].units = uds;

        // Incrementar el número de operaciones leídas
        n++;
    }

    // Cerrar el fichero
    if (fclose(fd) == -1) {
        perror("Error. No se pudo cerrar el fichero.\n");
        free(lista_operaciones);
        return -1;
    }

    // Comprobar que no haya menos operacion
    if (n < max_op_file) {
        perror("Error. Número de operaciones leidas < número máximo de operaciones indicado.\n");
        free(lista_operaciones);
        return -1;
    }

    // Inicializar la cola circular
    struct queue *cola_circular = queue_init(b_size);
    if (cola_circular == NULL) {
        perror("Error. No se pudo inicializar la cola.\n");
        return -1;
    }

    int op_actual;      // para establecer el orden de las operaciones

    // Productores y consumidores
    pthread_t productores[n_prod];
    pthread_t consumidores[n_cons];

    // Reparto equitativo de las operaciones entre los productores
    int n_op_prod = max_op_file / n_prod;
    int op_rest_prod = max_op_file % n_prod;
    op_actual = 0;
    for (int i = 0; i < n_prod; i++) {
        int n_op_actual = n_op_prod;
        if (op_rest_prod > 0) {
            n_op_actual++;
            op_rest_prod--;
        }

        // Argumentos para el hilo productor
        arg_productor *arg = malloc(sizeof(arg_productor));
        if (arg == NULL) {
            perror("Error. Reserva de memoria para los argumentos.\n");
            return -1;
        }
        // Array con las posiciones de las operaciones que le toca realizar
        int *ids = malloc(n_op_actual * sizeof(int));
        if (ids == NULL) {
            perror("Error. Reserva de memoria para los ids.\n");
            return -1;
        }
        for (int j = 0; j < n_op_actual; j++) {
            ids[j] = op_actual;
            op_actual++;
        }
        // Asignar los argumentos
        arg->ids = ids;
        arg->buffer = cola_circular;
        arg->max_op = n_op_actual;
        arg->list_operations = lista_operaciones;

        // Crear hilo productor
        if (pthread_create(&productores[i], NULL, &productor, arg) != 0) {
            perror("Error. Creación del hilo productor.\n");
            return -1;
        }
    }

    // Reparto equitativo de las operaciones entre los consumidores
    int n_op_cons = max_op_file / n_cons;
    int op_rest_cons = max_op_file % n_cons;
    op_actual = 0;
    for (int i = 0; i < n_cons; i++) {
        int n_op_actual = n_op_cons;
        if (op_rest_cons > 0) {
            n_op_actual++;
            op_rest_cons--;
        }

        // Argumentos para el hilo consumidor
        arg_consumidor *arg = malloc(sizeof(arg_consumidor));
        if (arg == NULL) {
            perror("Error. Reserva de memoria para los argumentos.\n");
            return -1;
        }
        // Array con las posiciones de las operaciones que le toca realizar
        int *ids = malloc(n_op_actual * sizeof(int));
        if (ids == NULL) {
            perror("Error. Reserva de memoria para los ids.\n");
            return -1;
        }
        for (int j = 0; j < n_op_actual; j++) {
            ids[j] = op_actual;
            op_actual++;
        }
        // Asignar los argumentos
        arg->ids = ids;
        arg->buffer = cola_circular;
        arg->max_op = n_op_actual;
        arg->profit = &profit;
        arg->product_stock = product_stock;

        // Crear hilo consumidor
        if (pthread_create(&consumidores[i], NULL, &consumidor, arg) != 0) {
            perror("Error. Creación del hilo consumidor.\n");
            return -1;
        }
    }

    // Esperar a que terminen los hilos productores
    for (int i = 0; i < n_prod; i++) {
        if (pthread_join(productores[i], NULL) != 0) {
            perror("Error. No se pudo esperar al hilo productor.\n");
            return -1;
        }
    }

    // Esperar a que terminen los hilos consumidores
    for (int i = 0; i < n_cons; i++) {
        if (pthread_join(consumidores[i], NULL) != 0) {
            perror("Error. No se pudo esperar al hilo consumidor.\n");
            return -1;
        }
    }

    // Output
    printf("Total: %d euros\n", profit);
    printf("Stock: \n");
    for (int i = 0; i < 5; i++) {
        printf("\tProduct %d: %d\n", i+1, product_stock[i]);
    }

    // Liberar memoria
    free(lista_operaciones);
    queue_destroy(cola_circular);

    // Destruir los mutex y las variables de condición
    pthread_mutex_destroy(&mutex_prod);
    pthread_mutex_destroy(&mutex_cons);
    pthread_cond_destroy(&cond_prod);
    pthread_cond_destroy(&cond_cons);

    return 0;
}