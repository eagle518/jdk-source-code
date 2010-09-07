/* Generic program structure for establishing
   connection-oriented client-server environment. */

#include "socket1common.h"

const char * const fileNameFQ = "/tmp/billb";
const char * const fileNameAB = "billb_01";

int getSockAddrSUN(struct sockaddr_un *sockaddr, int *adrlen, int protocol, const char *fileName, int abstract)
{
  int sock;

  memset(sockaddr, 0, sizeof(*sockaddr));
  sockaddr->sun_family = AF_UNIX;
  if(abstract) {
      // abstract namespace + rest of filename
      sockaddr->sun_path[0] = '\0';
      strncpy (sockaddr->sun_path+1, fileName, sizeof(sockaddr->sun_path)-2);
  } else {
      strncpy (sockaddr->sun_path, fileName, sizeof(sockaddr->sun_path)-1);
  }
  *adrlen = SUN_LEN (sockaddr);

  sock = socket(AF_UNIX, SOCK_STREAM, protocol);
  if (sock < 0) {
    printf("server socket failure %d\n", errno);
    perror("server: ");
    return -1;
  }
  return sock;
}  

