/* Generic program structure for establishing
   connection-oriented client-server environment. */

#ifndef _SOCKET1COMMON_H
#define _SOCKET1COMMON_H

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#ifndef SUN_LEN
# define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)        \
              + strlen ((ptr)->sun_path))
#endif

extern const char * const fileNameFQ;
extern const char * const fileNameAB;

int getSockAddrSUN(struct sockaddr_un *sockaddr, int *adrlen, int protocol, const char *fileName, int abstract);

#endif
