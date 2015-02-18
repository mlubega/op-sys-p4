#ifndef __REQUEST_H__
#define __REQUEST_H__
#include "cs537.h"
#include "connfd.h"

void requestHandle(file_info_t *fd);
void requestReadhdrs(rio_t *rp);
int requestParseURI(char *uri, char *filename, char *cgiargs);



#endif
