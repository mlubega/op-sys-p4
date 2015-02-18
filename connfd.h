#ifndef CONNFD_H
#define CONNFD_H

#include "cs537.h"
#include "request.h"
#include <stdlib.h>
#include <stdio.h>


typedef struct __fileinfo {

	int connfd;
	int is_static;
	struct stat sbuf;
	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	char filename[MAXLINE], cgiargs[MAXLINE];
	rio_t rio;

} file_info_t;

#endif
