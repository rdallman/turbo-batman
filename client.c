
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg) {
  perror(msg);
  exit(0);
}

int main(int argc, char *argv[]) {

  int sockfd,n;
  struct sockaddr_in servaddr,cliaddr;
  struct hostent *server;
  char sendline[1000];
  bzero(sendline, 1000);
  //maybe bigger?

  if (argc < 4)
  {
      printf("usage: %s server port reqID host...");
      exit(1);
  }

  sockfd=socket(AF_INET,SOCK_DGRAM,0);
  server = gethostbyname(argv[1]);
  if (server == NULL) 
    error("ERROR no such host");

  bzero(&servaddr,sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  bcopy((char*)server->h_addr,
        (char*)&servaddr.sin_addr.s_addr,
        server->h_length);
  servaddr.sin_port=htons(atoi(argv[2]));

  //FIXME
  int checksum = 0;

  //GID
  sendline[3] = 14;
  checksum += sendline[3];

  //RID 
  sendline[4] = atoi(argv[3]);
  checksum += sendline[4];

  //hosts
  int arg = 4;
  int index = 5;
  while (arg < argc) {
    //this may not be safe
    int j = 0;
    sendline[index++] = '~';
    checksum += sendline[index-1];
    while (argv[arg][j]) {
      sendline[index++] = argv[arg][j++];
      checksum += sendline[index-1];
    }
    arg++;
  }
  sendline[index] = '\0';

  sendline[0] = (index >> 8) & 0xFF;
  sendline[1] = index & 0xFF;
  checksum += sendline[0];
  checksum += sendline[1];

  //TODO compute checksum?
  sendline[2] = ~checksum;

  //FIXME temp for seeing what's up
  int i = 0;
  while (i < 5) {
    printf("%d ", sendline[i++]);
  }
  while (sendline[i]) {
    printf("%c", sendline[i++]);
  }
  printf("\n");

  sendto(sockfd,sendline,index,0,
                   (struct sockaddr *)&servaddr,sizeof(servaddr));

  char recvline[index];
  n=recvfrom(sockfd,recvline,index,0,NULL,NULL);

  //TODO only if valid
  printf("Request %d from group %d:\n", recvline[4], recvline[3]);
  i = 5;
  arg = 4;
  while (i < n) {
    printf("%s: %d.%d.%d.%d\n", argv[arg++],
        0xFF & recvline[i], 0xFF & recvline[i+1], 
        0xFF & recvline[i+2], 0xFF & recvline[i+3]);
    i += 4;
  }
}
