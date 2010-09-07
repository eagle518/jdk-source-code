/* Generic program structure for establishing
   connection-oriented client-server environment. */

/* client program */

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "socket1common.h"

int main(int argc, char * argv[])
{
  char bufIn[80];
  char bufOut[80];
  struct sockaddr_un myname;
  int sock, adrlen, cnt, i;

  // sock = getSockAddrSUN(&myname, &adrlen, 0, fileNameAB, 1);
  sock = getSockAddrSUN(&myname, &adrlen, 0, fileNameFQ, 0);
  if (sock < 0) {
    exit(1);
  }

  if (connect( sock, (struct sockaddr *)&myname, adrlen) < 0) {
    printf("client connect failure %d\n", errno);
    perror("client: ");
    exit(1);
  }

  printf("Connected to server - Socket address is %d is %x, <%s>\n",
      getpid(), myname.sun_path, myname.sun_path);

  snprintf(bufOut, sizeof(bufOut), "Client Message (pid %d)", getpid());
  if ( (cnt = write(sock, bufOut, strlen(bufOut)+1)) != strlen(bufOut)+1 ) {
      printf("Error sending to server: send %d - should send %d <%s>\n", cnt, strlen(bufOut), bufOut);
  } else {
      printf("Send to server: %d <%s>\n", cnt, bufOut);
  }

  // blocking message read ..
  printf("Waiting for message from server ...\n");
  memset(bufIn, 0, sizeof(bufIn));
  cnt=0;
  while( (i=read(sock, bufIn+cnt, sizeof(bufIn)-cnt-1))>0 ) {
      printf ("Client got message fragment: %d\n", i);
      cnt += i;
      if ( bufIn[cnt-1] == '\0' ) break; // end of message reached
  }
  bufIn[cnt-1] = '\0'; // EOS just to be sure ..
  printf ("Client with pid %d got message: %d <%s>\n", getpid(), cnt, bufIn);

  close(sock);

  return 0;
}


