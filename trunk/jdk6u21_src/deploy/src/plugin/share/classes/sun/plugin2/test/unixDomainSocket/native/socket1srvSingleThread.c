/* Generic program structure for establishing
   connection-oriented client-server environment. */

/* server program - single thread ..*/


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
  struct sockaddr_un srvSockAddr;
  struct sockaddr_un clientSockAddr;
  int sock, new_sd, srvAddrlen, clientAddrlen, cnt, i;

  // sock = getSockAddrSUN(&srvSockAddr, &srvAddrlen, 0, fileNameAB, 1);
  sock = getSockAddrSUN(&srvSockAddr, &srvAddrlen, 0, fileNameFQ, 0);
  if (sock < 0) {
    exit(1);
  }

  if (bind(sock, (struct sockaddr *)&srvSockAddr, srvAddrlen) < 0) {
    printf("server bind failure %d\n", errno);
    perror("server: ");
    exit(1);
  }

  if (listen(sock, 1) < 0) {
    printf("server listen failure %d\n", errno);
    perror("server: ");
    exit(1);
  }

  printf("Socket address in server %d is %x, <%s>\n", getpid(), srvSockAddr.sun_path, srvSockAddr.sun_path);

  /*  Place the server in an infinite loop, waiting
      on connection requests to come from clients.
      In practice, there would need to be a clean
      way to terminate this process, but for now it
      will simply stay resident until terminated by
      the starting terminal or the super-user.        */

  while (1) {
    clientAddrlen=sizeof(struct sockaddr_un);
    memcpy(&clientSockAddr, &srvSockAddr, srvAddrlen);
    printf("Waiting for client to connect ...\n");
    if ( (new_sd = accept(sock, (struct sockaddr *)&clientSockAddr, &clientAddrlen)) < 0) {
      printf("server accept failure %d\n", errno);
      perror("server: ");
      exit(1);
    }

    printf("Client connected - Socket address to client is %d is %x, <%s>\n",
      getpid(), clientSockAddr.sun_path, clientSockAddr.sun_path);

      // blocking message read ..
      printf("ServerThread with pid %d, waiting for message from client ...\n", getpid());
      memset(bufIn, 0, sizeof(bufIn));
      cnt=0;
      while( (i=read(new_sd, bufIn+cnt, sizeof(bufIn)-cnt-1))>0 ) {
          printf ("ServerThread with pid %d got message fragment: %d\n", getpid(), i);
          cnt += i;
          if ( bufIn[cnt-1] == '\0' ) break; // end of message reached
      }
      bufIn[cnt-1] = '\0'; // EOS just to be sure ..
      printf ("ServerThread with pid %d got message: %d <%s>\n", getpid(), cnt, bufIn);

      snprintf(bufOut, sizeof(bufOut)-1, "Server (pid %d) echo: '%s'", getpid(), bufIn);
      bufOut[sizeof(bufOut)-1] = '\0' ;
      printf ("ServerThread with pid %d returns message: %d <%s>\n", getpid(), strlen(bufOut), bufOut);
      cnt = write(new_sd, bufOut, strlen(bufOut)+1); // incl. EOS
      printf("ServerThread with pid %d closing client connection\n", getpid());

      close (new_sd); /* close prior to exiting  */

  }  /* closing bracket for while (1) ... )    */

  close (sock);  /* child does not need it  */
  unlink(fileNameFQ);
  return 0;

}  

