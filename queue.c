#include "queue.h"
#include <assert.h>
#include <stdlib.h>
#include "connfd.h"
/*
 *Queue.c Concurrent queue
 *
 *-Uses lock for enqueue and dequeue operations
 *-Enqueue keeps list in sorted order based on an
 * integer key
 *
 *
 * */



//intialize head node and lock
void queue_init (queue_t *q){

	//empty header node to simplify enqueue/dequeue ops
	q->head =(queue_node_t *) malloc(sizeof(queue_node_t));
	(q->head)->next = NULL;
	pthread_mutex_init(&q->head_lock, NULL);

}

//enqueue in increasing order by key 
void enqueue(queue_t *q, file_info_t * finfo, int key) 

{ 
	//create a new node
	queue_node_t * new_node = (queue_node_t *)  malloc(sizeof(queue_node_t)); 
	assert(new_node != NULL);
	new_node->finfo =finfo; 
	new_node->key = key;
	new_node->next = NULL; 
	
	//grab lock, insert node, release lock
	pthread_mutex_lock(&q->head_lock);

		 queue_node_t *temp = q->head;
		 while( (temp->next != NULL) &&  (temp->next)->key <=  new_node->key)
			 temp = temp->next;
		 
		 new_node->next = temp->next;
		 temp->next = new_node;
		printf("After Enqueue:  "); 
		print(q);
	pthread_mutex_unlock(&q->head_lock);

}

//dequeue from front only
//int dequeue(queue_t *q, file_info_t * data_struct)
file_info_t*  dequeue(queue_t *q, file_info_t * data_struct)

{ 

	pthread_mutex_lock(&q->head_lock);
	
	queue_node_t * tempPtr;
       	queue_node_t * headPtr;	
	tempPtr = q->head; 
	headPtr = tempPtr->next;
	
	data_struct = headPtr->finfo;
	printf("In Dequeue, connfd is: %d\n", data_struct->connfd);
//	printf("In Dequeue, connfd is: %d\n", data_struct->connfd);
//	printf("In Dequeue, connfd is: %d\n", data_struct->connfd);
//	printf("In Dequeue, connfd is: %d\n", data_struct->connfd);
//	printf("In Dequeue, connfd is: %d\n", data_struct->connfd);
	
	
	
	q->head = headPtr; 
	printf("After Dequeue   ");
	print(q);
	pthread_mutex_unlock(&q->head_lock);
	free (tempPtr); 
	return data_struct; 

} 

int isEmpty(queue_t * q) 
{ 
	return (q->head)->next ==  NULL; 
} 
//called within queue.c only--does not support 
//concurrent printing when called outside of 
//queue.c
void print (queue_t *q){
	int i = 0;

//	pthread_mutex_lock(&q->head_lock);
	queue_node_t *temp = q->head;
	printf("QUEUE: ");
	while (temp->next != NULL){
		temp = temp->next;
		printf("[Node %d : connfd %d  key %d] next--> ", i, (temp->finfo)->connfd, temp->key);
		i++;
	}	
	printf("\n\n");
//	pthread_mutex_unlock(&q->head_lock);
}
