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

/* Variable global */
struct queue *buffer_circular;

/* Mutex */
pthread_mutex_t mutex_prod = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cons = PTHREAD_MUTEX_INITIALIZER;

/* Coste y precio de cada producto */
int coste[5] = {2, 5, 15, 25, 100};
int precio[5] = {3, 10, 20, 40, 125};

/* Argumentos del hilo productor */
typedef struct arg_productor {
    int *ids;
    int max_op;
    struct element *list_operations;
} arg_productor;

/* Hilo productor */
void *productor(void *arg) {
    // Obtener los argumentos
    arg_productor *args = (arg_productor *) arg;
    // Obtener los índices a los elementos a los que tiene que acceder
    int *ids = args->ids;
    // Bucle para leer las operaciones
    int n_op = 0;
    while (n_op < args->max_op) {
        // Bloquear el mutex del productor
        pthread_mutex_lock(&mutex_prod);
        // Crear la operación
        struct element operacion;
        operacion.product_id = args->list_operations[ids[n_op]].product_id;
        operacion.op = args->list_operations[ids[n_op]].op;
        operacion.units = args->list_operations[ids[n_op]].units;
        // Insertar la operación en la cola
        queue_put(buffer_circular, &operacion);
        n_op++;
        // Desbloquear el mutex del productor
        pthread_mutex_unlock(&mutex_prod);
    }
    // Finalizar el hilo
    pthread_exit(NULL);
}

/* Argumentos del hilo consumidor */
typedef struct arg_consumidor {
    int max_op;
    int profit;
    int *product_stock;
} arg_consumidor;

/* Hilo consumidor */
void *consumidor(void *arg) {
    // Obtener los argumentos
    arg_consumidor *args = (arg_consumidor *) arg;
    // Obtener los argumentos
    int max_op = args->max_op;
    int profit = args->profit;
    int *product_stock = args->product_stock;
    // Bucle para leer las operaciones
    int n_op = 0;
    while (n_op < max_op) {
        // Bloquear el mutex del consumidor
        pthread_mutex_lock(&mutex_cons);
        // Extraer la operación de la cola
        struct element *operacion = queue_get(buffer_circular);
        // La operación es de compra (PURCHASE)
        if (operacion->op == 0) {
            // Actualizar el stock
            product_stock[operacion->product_id] += operacion->units;
            // Actualizar el beneficio
            profit -= coste[operacion->product_id] * operacion->units;
        } 
        // La operación es de venta (SALE)
        else {
            // Comprobar si hay suficiente stock
            if (product_stock[operacion->product_id] >= operacion->units) {
                // Actualizar el stock
                product_stock[operacion->product_id] -= operacion->units;
                // Actualizar el beneficio
                profit += precio[operacion->product_id] * operacion->units;
            }
        }
        n_op++;
        // Desbloquear el mutex del consumidor
        pthread_mutex_unlock(&mutex_cons);
    }
    
    // Finalizar el hilo
    pthread_exit(NULL);
}

/* Función principal */
int main (int argc, const char *argv[]) {
    int profit = 0;
    int product_stock[5] = {0, 0, 0, 0, 0};

    // Comprobar el número de argumentos
    if (argc != 5) {
        perror("Error: El número de argumentos incorrecto.\n");
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
    // Abrir el fichero de entrada
    FILE *fd;
    if ((fd = fopen(argv[1], "r")) == NULL) {
        perror("Error: No se pudo abrir el fichero.\n");
        return -1;
    }
    // Leer el número de operaciones, la primera linea del fichero
    int max_op_file;
    if (fscanf(fd, "%d", &max_op_file) != 1) {
        perror("Error: No se pudo leer el número de operaciones.\n");
        return -1;
    }
    // Reservar memoria para todas las operaciones
    struct element *list_operations;
    list_operations = (struct element *) malloc(max_op_file * sizeof(struct element));
    // Almacenar todas las operaciones en la lista
    int n_read = 0;
    for (int i=0; i<max_op_file; i++) {
        int product_id, units;
        char *str_op = "";
        if (fscanf(fd, "%d %s %d", &product_id, str_op, &units) != 3) {
            perror("Error: No se pudo leer la operación.\n");
            return -1;
        }
        list_operations[i].product_id = product_id;
        // Operacion
        if (strcmp(str_op, "PURCHASE")) {
            list_operations[i].op = 0;
        } else  if (strcmp(str_op, "SALE")) {
            list_operations[i].op = 1;
        } else {
            perror("Error: Operación no válida.\n");
            return -1;
        }
        list_operations[i].units = units;
        n_read++;
    }
    // Comprobar si el numero de operaciones es correcto
    if (n_read < max_op_file) {
        perror("Error: El número de operaciones no es válido.\n");
        return -1;
    }
    // Inicializar la cola
    buffer_circular = queue_init(size_cola);
    if (buffer_circular == NULL) {
        return -1;
    }
    // Crear los productores
    pthread_t productores[n_productores];
    // Repartir las operaciones entre los productores
    int n_op = max_op_file / n_productores;
    int ids[n_productores];
    for (int i=0; i<n_productores; i++) {
        ids[i] = i * n_op;
        arg_productor args;
        args.ids = ids;
        args.max_op = n_op;
        args.list_operations = list_operations;
        pthread_create(&productores[i], NULL, productor, (void *) &args);
    }
    
    // Crear los consumidores
    pthread_t consumidores[n_consumidores];
    // Repartir las operaciones entre los consumidores
    n_op = max_op_file / n_consumidores;
    for (int i=0; i<n_consumidores; i++) {
        arg_consumidor args;
        args.max_op = n_op;
        args.profit = profit;
        args.product_stock = product_stock;
        pthread_create(&consumidores[i], NULL, consumidor, (void *) &args);
    }
    
    // Esperar a que terminen los productores
    for (int i=0; i<n_productores; i++) {
        pthread_join(productores[i], NULL);
    }

    // Esperar a que terminen los consumidores
    for (int i=0; i<n_consumidores; i++) {
        pthread_join(consumidores[i], NULL);
    }

    // Output
    printf("Total: %d euros\n", profit);
    printf("Stock:\n");
    for (int i=0; i<5; i++) {
        printf("Producto %d: %d uds\n", i+1, product_stock[i]);
    }

    // Liberar memoria
    free(list_operations);
    queue_destroy(buffer_circular);
    fclose(fd);

    return 0;
}
