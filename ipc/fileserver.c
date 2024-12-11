#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>

#define PORT 8080
#define BSIZE 1024

int main() {
    int serverFd, newSocket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BSIZE];
    ssize_t bytesRead, bytesSent;

    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    if (listen(serverFd, 3) < 0) {
        perror("Listen failed");
        close(serverFd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        newSocket = accept(serverFd, (struct sockaddr *)&address, &addrlen);
        if (newSocket < 0) {
            perror("Accept failed");
            continue;
        }

        memset(buffer, 0, BSIZE);
        ssize_t filenameLen = read(newSocket, buffer, BSIZE - 1);
        if (filenameLen <= 0) {
            perror("Failed to read filename");
            close(newSocket);
            continue;
        }

        buffer[filenameLen] = '\0'; // Null-terminate the filename
        printf("Client requested file: %s\n", buffer);

        int fileFd = open(buffer, O_RDONLY);
        if (fileFd < 0) {
            perror("Error opening file");
            close(newSocket);
            continue;
        }

        printf("Sending file data...\n");
        while ((bytesRead = read(fileFd, buffer, BSIZE)) > 0) {
            bytesSent = write(newSocket, buffer, bytesRead);
            if (bytesSent < 0) {
                perror("Failed to send file content");
                break;
            }
        }
        close(fileFd);

        printf("Receiving updated file from client...\n");
        int updatedFileFd = open(buffer, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (updatedFileFd < 0) {
            perror("Failed to create updated file");
            close(newSocket);
            continue;
        }

        while ((bytesRead = read(newSocket, buffer, BSIZE)) > 0) {
            write(updatedFileFd, buffer, bytesRead);
        }
        close(updatedFileFd);

        printf("Updated file saved.\n");
        close(newSocket);
    }

    close(serverFd);
    return 0;
}
