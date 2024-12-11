#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/stat.h>

const int port = 8080;
const int bsize = 1024;

int main() {
    int serverFd, newSocket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[bsize];
    ssize_t bytesRead;

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
        buffer[filenameLen] = '\0';
        printf("Client requested file: %s\n", buffer);

        int fileFd = open(buffer, O_RDONLY);
        if (fileFd < 0) {
            perror("Error opening file");
            close(newSocket);
            continue;
        }

        while ((bytesRead = read(fileFd, buffer, bsize)) > 0) {
            write(newSocket, buffer, bytesRead);
        }
        close(fileFd);
        shutdown(newSocket, SHUT_WR);
        printf("File sent to client. Waiting for updated file...\n");

        int updatedFd = open("text.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

        while ((bytesRead = read(newSocket, buffer, bsize)) > 0) {
            write(updatedFd, buffer, bytesRead);
        }
        close(updatedFd);

        printf("Updated file received and saved as '%s'.\n", buffer);
        close(newSocket);
    }

    close(serverFd);
    return 0;
}
