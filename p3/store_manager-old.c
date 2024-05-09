/* SSOO-P3 23/24 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <pthread.h>
#include "queue.h"
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>


/* Mutex */
pthread_mutex_t mutex_prod = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cons = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
bool productores_terminados = false;

/* Coste y precio de cada producto */
int coste[5] = {2, 5, 15, 25, 100};
int precio[5] = {3, 10, 20, 40, 125};

/* Argumentos del hilo productor */
typedef struct arg_productor {
    struct queue *buffer;
    int *ids;
    int max_op;
    struct element *list_operations;
} arg_productor;

/* Hilo productor */
void *productor(void *arg) {
    printf("Productor\n");
    // Obtener los argumentos
    arg_productor *args = (arg_productor *) arg;
    // Obtener la cola
    queue *buffer_circular = args->buffer;
    // Obtener los índices a los elementos a los que tiene que acceder
    int *ids = args->ids;
    // Bucle para leer las operaciones
    int n_op = 0;
    //printf("max_op prod: %d\n", args->max_op);
    while (n_op < args->max_op) {
        //printf("n_op prod: %d\n", n_op);
        // Bloquear el mutex del productor
        pthread_mutex_lock(&mutex_prod);
        // Crear la operación
        struct element *operacion;
        // Reservo memoria
        operacion = (struct element *) malloc(sizeof(struct element));
        if (operacion == NULL) {
            perror("Error: Reservar memoria para la operación.\n");
            return NULL;
        }
        // Asignar los valores de la operación
        operacion->product_id = args->list_operations[ids[n_op]].product_id;
        operacion->op = args->list_operations[ids[n_op]].op;
        operacion->units = args->list_operations[ids[n_op]].units;
        // Insertar la operación en la cola
        queue_put(buffer_circular, operacion);
        n_op++;
        // Imprimir la cola
        print_queue(buffer_circular);

        productores_terminados = true;
        pthread_cond_signal(&cond);

        // Desbloquear el mutex del productor
        pthread_mutex_unlock(&mutex_prod);
    }
    // Liberar array de ids    
    free(ids);
    free(args);

    printf("Fin productor\n");

    // Finalizar el hilo
    pthread_exit(NULL);
}

/* Argumentos del hilo consumidor */
typedef struct arg_consumidor {
    struct queue *buffer;
    int max_op;
    int *profit;
    int *product_stock;
} arg_consumidor;

/* Hilo consumidor */
void *consumidor(void *arg) {
    printf("Consumidor\n");
    // Obtener los argumentos
    arg_consumidor *args = (arg_consumidor *) arg;
    // Obtener los argumentos
    queue *buffer_circular = args->buffer;
    /* int max_op = args->max_op; */
    int *profit = args->profit;
    int *product_stock = args->product_stock;
    // Bucle para leer las operaciones
    int n_op = 0;
    //printf("max_op cons: %d\n", max_op);
    while (/* n_op < max_op ||  */ !productores_terminados && !queue_empty(buffer_circular)) {
        //printf("n_op cons: %d\n", n_op);
        // Bloquear el mutex del consumidor
        pthread_mutex_lock(&mutex_cons);
        // Extraer la operación de la cola
        struct element *operacion = queue_get(buffer_circular);
        // La operación es de compra (PURCHASE)
        if (operacion->op == 0) {
            // Actualizar el stock
            product_stock[operacion->product_id-1] += operacion->units;
            // printf("CAmbio en el producto: %d y su stock es: %d\n", operacion->product_id-1, product_stock[operacion->product_id-1]);
            // Actualizar el beneficio
            *profit -= coste[operacion->product_id-1] * operacion->units;
        } 
        // La operación es de venta (SALE)
        else if (operacion->op == 1) {
            // Comprobar si hay suficiente stock
            if (product_stock[operacion->product_id-1] >= operacion->units) {
                // Actualizar el stock
                product_stock[operacion->product_id-1] -= operacion->units;
                // Actualizar el beneficio
                *profit += precio[operacion->product_id-1] * operacion->units;
            }
        }
        n_op++;
        free (operacion);

        // Imprimir la cola
        //print_queue(buffer_circular);
        
        // Desbloquear el mutex del consumidor
        pthread_mutex_unlock(&mutex_cons);
    }

    // Liberar memoria
    free(args);

    printf("Fin consumidor\n");
    
    // Finalizar el hilo
    pthread_exit(NULL);
}

/* Función principal */
int main (int argc, const char *argv[]) {
    int profit = 0;
    int product_stock[5] = {0, 0, 0, 0, 0};

    // Comprobar el número de argumentos
    if (argc != 5) {
        perror("Error: La operacion es ./store_manager <file> <N_prods> <N_cons> <B_size>.\n");
        return -1;
    }
    // Comprobar los valores de los argumentos
    if (atoi(argv[2]) <= 0 || atoi(argv[3]) <= 0 || atoi(argv[4]) <= 0) {
        perror("Error: Valores de los argumentos no válidos.\n");
        return -1;
    }
    int n_productores = atoi(argv[2]);
    int n_consumidores = atoi(argv[3]);
    int size_cola = atoi(argv[4]);
    // printf("argumentos correctos!\n");
    // Abrir el fichero de entrada
    FILE *fd;
    if ((fd = fopen(argv[1], "r")) == NULL) {
        perror("Error: No se pudo abrir el fichero.\n");
        return -1;
    }
    // printf("Fichero abierto\n");
    // Leer el número de operaciones, la primera linea del fichero
    int max_op_file;
    if (fscanf(fd, "%d", &max_op_file) != 1) {
        perror("Error: No se pudo leer el número de operaciones.\n");
        return -1;
    }
    // printf("Número máx op: %d\n", max_op_file);
    // Reservar memoria para todas las operaciones
    struct element *list_operations;
    list_operations = (struct element *) malloc(max_op_file * sizeof(struct element));
    if (list_operations == NULL) {
        perror("Error: Reservar memoria para las operaciones.\n");
        return -1;
    }
    // printf("Memoria reservada para las operaciones\n");
    // Almacenar todas las operaciones en la lista
    int n_read = 0;
    int i = 0;
    while (i <= max_op_file) {
        if (feof(fd)) {
            break;
        }
        int product_id, units;
        char str_op[10];
        if (fscanf(fd, "%d %s %d", &product_id, str_op, &units) != 3) {
            break;
        }

        // printf("La linea leida es: %d %s %d\n", product_id, str_op, units);

        list_operations[i].product_id = product_id;
        // Operacion
        if (strcmp(str_op, "PURCHASE") == 0) {
            list_operations[i].op = 0;
        } else  if (strcmp(str_op, "SALE") == 0) {
            list_operations[i].op = 1;
        } else {
            perror("Error: Operación no válida.\n");
            return -1;
        }

        // printf("La linea guardada es: %d %d %d\n", list_operations[i].product_id, list_operations[i].op, list_operations[i].units);

        list_operations[i].units = units;
        n_read++;

        i++;
    }
    // printf("Operaciones leídas y almacenadas en el array\n");
    // Comprobar si el numero de operaciones es correcto
    if (n_read < max_op_file) {
        perror("Error: El número de operaciones no es válido.\n");
        return -1;
    }
    // Inicializar la cola
    queue *buffer;
    buffer = queue_init(size_cola);
    if (buffer == NULL) {
        return -1;
    }
    // printf("Cola inicializada\n");

    // Crear los productores
    pthread_t productores[n_productores];
    // Repartir las operaciones entre los productores
    int n_op = max_op_file / n_productores;
    for (int i=0; i<n_productores; i++) {
        int *ids = malloc(n_op * sizeof(int));
        if (ids == NULL) {
            perror("Error: No se pudo reservar memoria para los ids.\n");
            return -1;
        }
        for (int j=0; j<n_op; j++) {
            ids[j] = i * n_op + j;
        }
        arg_productor *args = malloc(sizeof(arg_productor));
        args->buffer = buffer;
        args->ids = ids;
        args->max_op = n_op;
        args->list_operations = list_operations;
        pthread_create(&productores[i], NULL, &productor, (void *) args);
    }

    printf("Productores creados\n");
    
    // Crear los consumidores
    pthread_t consumidores[n_consumidores];
    // Repartir las operaciones entre los consumidores
    n_op = max_op_file / n_consumidores;
    for (int i=0; i<n_consumidores; i++) {
        arg_consumidor *args = malloc(sizeof(arg_consumidor));
        args->buffer = buffer;
        args->max_op = n_op;
        args->profit = &profit;
        args->product_stock = product_stock;
        pthread_create(&consumidores[i], NULL, &consumidor, (void *) args);
    }

    printf("Consumidores creados\n");
    
    // Esperar a que terminen los productores
    for (int i=0; i<n_productores; i++) {
        pthread_join(productores[i], NULL);
    }
    printf("Productores fin\n");

    // Esperar a que terminen los consumidores
    for (int i=0; i<n_consumidores; i++) {
        pthread_join(consumidores[i], NULL);
    }
    printf("Consumidores fin\n");

    // Output
    printf("Total: %d euros\n", profit);
    printf("Stock:\n");
    for (int i=0; i<5; i++) {
        printf("Producto %d: %d uds\n", i+1, product_stock[i]);
    }

    // Liberar memoria
    pthread_mutex_destroy(&mutex_prod);
    pthread_mutex_destroy(&mutex_cons);
    free(list_operations);
    queue_destroy(buffer);
    fclose(fd);

    return 0;
}
