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


int main (int argc, const char * argv[]) {
    /*
    int profits = 0;
    int product_stock [5] = {0};

    // Output
    printf("Total: %d euros\n", profits);
    printf("Stock:\n");
    printf("  Product 1: %d\n", product_stock[0]);
    printf("  Product 2: %d\n", product_stock[1]);
    printf("  Product 3: %d\n", product_stock[2]);
    printf("  Product 4: %d\n", product_stock[3]);
    printf("  Product 5: %d\n", product_stock[4]);
    */

    printf("Iniciliazando cola...\n");
    queue *q = queue_init(5);
    printf("Añadiendo elementos a la cola...\n");
    struct element *elem = (struct element *) malloc(sizeof(struct element));
    elem->product_id = 1;
    elem->op = 1;
    elem->units = 1;
    queue_put(q, elem);
    print_queue(q);
    elem = (struct element *) malloc(sizeof(struct element));
    elem->product_id = 2;
    elem->op = 2;
    elem->units = 2;
    queue_put(q, elem);
    print_queue(q);
    
    return 0;
}
