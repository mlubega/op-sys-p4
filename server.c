#include "cs537.h"
#include "request.h"
#include <pthread.h>
#include <string.h>
#include "queue.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "connfd.h"
#define FIFO 0
#define SFNF 1
#define SFF 3


// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// CS537: Parse the new arguments too


//Variables


 pthread_mutex_t pool_lock;
 pthread_cond_t signal_master; //where workers threads sleep
 pthread_cond_t signal_worker; // where main thread sleeps
 pthread_t *workers; //array of worker threads-at initializations
 queue_t *buf; //queue to place workers
 int buf_count;

///string comparison for proper scheduling argument
char* fifo = "FIFO";
char* sml_name_first = "SFNF";
char* sml_file_first = "SFF";

//global variables for inititalization purposese
int worker_threads;
int request_limit;
char*  schedule;
int schdl;

//Wrappers-In Progress
/*

void Pthread_mutex_lock(pthread_mutex_t * mutex){
	int rc= pthread_mutex_lock(mutex);
	assert(rc == 0);

}
void Pthread_mutex_init(pthread_mutex_t * mutex){
	int rc= pthread_mutex_init(&mutex, NULL);
	assert(rc == 0);

}
void Pthread_mutex_unlock(pthread_mutex_t * mutex){
	int rc= pthread_mutex_unlock(mutex);
	assert(rc == 0);

}
void Pthread_mutex_lock(pthread_mutex_t * mutex){
	int rc= pthread_mutex_lock(mutex);
	assert(rc == 0);

}
*/



//put in buffer
void put(file_info_t * finfo, int key){
	
	enqueue(buf, finfo, key);
	buf_count++;
	printf("buf count: %d\n", buf_count);


}

///remove from buffer
//int get(file_info_t * temp){
file_info_t*  get(file_info_t * temp){
	
        temp  = dequeue(buf, temp);
	buf_count--;
	printf("buf count: %d\n", buf_count);
	return temp;
	
} 


//workers threads run here
void * consume(void * arg){


		file_info_t * retrieved_info = (file_info_t*) malloc(sizeof(file_info_t*));

	for(;;){

		pthread_mutex_lock(&pool_lock);
		while(buf_count == 0){
//			printf("Entered CV\n");
			pthread_cond_wait(&signal_master, &pool_lock);
//			printf("return from cv\n");
		}
//		printf("buffer has data---\n");

		retrieved_info = get(retrieved_info);
//		printf("data retrieved by worker, connfd: %d\n", retrieved_info->connfd);

		pthread_cond_signal(&signal_master);// signals next worker thread
		pthread_cond_signal(&signal_worker);// signals main thread
		pthread_mutex_unlock(&pool_lock);
		requestHandle(retrieved_info);
		Close(retrieved_info->connfd);
//		printf("End of Consume\n\n");
	}
	
	free(retrieved_info);
	return NULL;
}
//creates workers threads
void * create_workers(pthread_t * worker_array){
	int i = 0;
	for(i = 0; i < worker_threads ; i++){
		pthread_create(&(worker_array[i]), NULL, consume, NULL );
//		printf("worker thread %d created\n", i);
	}
	return NULL;
}


//initializes all vars needed
void thread_pool_init(){


	pthread_mutex_init(&pool_lock, NULL);
	pthread_cond_init(&signal_master, NULL);
	pthread_cond_init(&signal_worker, NULL);
	
	buf_count=0;
	buf = (queue_t*)malloc(sizeof(queue_t));//concurrent queue
	queue_init(buf);
	workers = (pthread_t*)malloc(worker_threads*sizeof(pthread_t));
//	buf = (int*)malloc(request_limit*sizeof(int));
	create_workers(workers);

}

//gets and checks command line arguements
void getargs(int *port, int argc, char *argv[])
{
	if (argc != 5) {
		fprintf(stderr, "Usage: %s <port> <threads> <buffers> <schedalg>\n", argv[0]);
		exit(1);
	}
	*port = atoi(argv[1]);

	worker_threads = atoi(argv[2]);
	if(worker_threads <= 0){
		fprintf(stderr, "must have a pos num of worker threads\n");
		exit(1);
	}

	request_limit = atoi(argv[3]);
	if(request_limit <= 0){
		fprintf(stderr, "must have pos num of requests\n");
		exit(1);
	}

	schedule = argv[4];
	int c;

	if((c = strncmp(schedule, fifo, sizeof(fifo)))  == 0)
		schdl = FIFO;
	else if ((c = strncmp(schedule, sml_name_first, sizeof(sml_name_first))) == 0)
		schdl = SFNF;
	else if ((c = strncmp(schedule, sml_file_first, sizeof(sml_file_first))) == 0)
		schdl = SFF;
	else{
		fprintf(stderr, "Invalid scheduling policy\n");
		exit(1);
	}

//	printf("worker threads: %d\n", worker_threads);
//	printf("request_limit: %d\n", request_limit);
//	printf("scheduling: %s\n", schedule);

}



int main(int argc, char *argv[])
{
	int listenfd, /* connfd,*/ port, clientlen;
	struct sockaddr_in clientaddr;

	getargs(&port, argc, argv);

	listenfd = Open_listenfd(port);
	thread_pool_init();		
	int order_recieved = 0;	
	int len_filename;
	int size_file;
	int key ;
	
	file_info_t * ptr_connfd;

/*	
        	
	//data to be place in file_info_t struct
        char buf[MAXLINE],method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	char filename[MAXLINE], cgiargs[MAXLINE];
	struct stat sbuf;
	rio_t rio;
	int is_static;
	int connfd;
*/	
	//main thread loops here
	for(;;){
		clientlen = sizeof(clientaddr);
		ptr_connfd = (file_info_t*)malloc(sizeof(file_info_t));
		ptr_connfd->connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
		
		//acquire lock
		pthread_mutex_lock(&pool_lock);
		
		//check buf count
		while(buf_count == request_limit)
			pthread_cond_wait(&signal_worker, &pool_lock);
		
	
		//parse out the filename and file size	

		 Rio_readinitb(&(ptr_connfd->rio), (ptr_connfd)->connfd);
		 Rio_readlineb(&(ptr_connfd->rio),ptr_connfd->buf, MAXLINE);
		 sscanf(ptr_connfd->buf, "%s %s %s", ptr_connfd->method, ptr_connfd->uri, ptr_connfd->version);
		 requestReadhdrs(&(ptr_connfd->rio));

		ptr_connfd->is_static  = requestParseURI(ptr_connfd->uri, ptr_connfd->filename, ptr_connfd->cgiargs);

//		how to check errors? less than zero, etc
		 len_filename = (int) strlen(ptr_connfd->filename);
		 stat(ptr_connfd->filename, &(ptr_connfd->sbuf));
		 size_file =(int) (ptr_connfd->sbuf).st_size;

		 //set key based on scheduling algo--key used to keep
		 // queue in sorted order
		 switch(schdl){

			 case FIFO:
				 key = order_recieved;
				 break;
			 case SFNF:
				 key = len_filename;
				 break;
			 case SFF:
				 key = size_file;
				 break;
			default:
  				fprintf(stderr, "Invalid scheduling policy\n");
		                 exit(1);
				break;
		}

		 
 
		printf("%s\n", ptr_connfd->filename);
		printf("string length: %d\n",len_filename);
		printf("file size: %d\n", size_file);


//		printf("Master thread to put connfd: %d, schdl %s, key value %d\n", connfd, schedule,  key);
		
		//place connfd in buffer (queue)
		put(ptr_connfd, key);
	
		
		//signal and release locks
		pthread_cond_signal(&signal_master);
		pthread_mutex_unlock(&pool_lock);

		order_recieved++; //variable for FIFO ordering
/*
		//clear out char arrays and structs to avoid data corruption
		memset(buf, 0, strlen(buf)+1);
		memset(method, 0, strlen(method)+1);
		memset(uri, 0, strlen(uri)+1);
		memset(version, 0, strlen(version)+1);
		memset(filename, 0, strlen(filename)+1);
		memset(cgiargs, 0, strlen(cgiargs)+1);
		rio = Empty_rio;
		sbuf = Empty_sbuf;
*/
	}
/*	while (1) {

		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
		put(connfd);
		requestHandle(connfd);
		Close(connfd);
	}
*/
}






