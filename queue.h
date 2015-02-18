#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "request.h"
#include "cs537.h"
#include "connfd.h"


typedef struct queue_node_t{

	file_info_t * finfo;	
	int key;
	struct queue_node_t * next;

}queue_node_t;


typedef struct queue_t{

	queue_node_t *head;
	pthread_mutex_t head_lock;

} queue_t;

void queue_init (queue_t *q);
void enqueue(queue_t  *q, file_info_t * finfo, int key);
//int dequeue(queue_t *q, file_info_t * fd);
file_info_t*  dequeue(queue_t *q, file_info_t * fd);
int isEmpty(queue_t * q);
void print(queue_t *q);
