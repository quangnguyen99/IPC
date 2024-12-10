// file_server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      
#include <arpa/inet.h>   
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>

const int port = 8080;
const int bsize = 1024;

int main() {
  int serverFd, newSocket;
  struct sockaddr_in address;
  socklen_t addrlen = sizeof(address);
  char buffer[bsize];
  ssize_t bytesRead, bytesSent;

  serverFd = socket(AF_INET, SOCK_STREAM, 0);

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  bind(serverFd, (struct sockaddr *)&address, addrlen);

  listen(serverFd, 3);

  printf("Server listening on port %d...\n", port);

  while (1) {
    newSocket = accept(serverFd, (struct sockaddr *)&address, &addrlen);

    memset(buffer, 0, bsize);
    ssize_t filenameLen = read(newSocket, buffer, bsize - 1);
    buffer[filenameLen] = '\0'; // Null-terminate the filename
    printf("Client requested file: %s\n", buffer);

    int fileFd = open(buffer, O_RDONLY);

    while ((bytesRead = read(fileFd, buffer, bsize)) > 0) {
      bytesSent = write(newSocket, buffer, bytesRead);
    }
    close(fileFd);
    close(newSocket);
    printf("Connection closed.\n");
  }

  close(serverFd);
  return 0;
}
