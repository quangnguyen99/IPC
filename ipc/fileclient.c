#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      
#include <arpa/inet.h>   
#include <sys/socket.h>
#include <sys/types.h>

const int port = 8080;
const int bsize = 1024;


int main(int argc, char *argv[]) {
    int sock = 0;
    struct sockaddr_in servAddr;
    char buffer[bsize];
    char *filename = argv[1];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);

    inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr);

    connect(sock, (struct sockaddr *)&servAddr, sizeof(servAddr));

    write(sock, filename, strlen(filename)); // sending the filename

    printf("Contents of '%s':\n", filename);
    while (1) {
      ssize_t bytesReceived = read(sock, buffer, bsize - 1); // reading the file

        if (bytesReceived == 0) {
            // No more data
            break;
        }
        buffer[bytesReceived] = '\0'; // Null-terminate the buffer
        printf("%s", buffer);
    }
    printf("\n");

    close(sock);
    return 0;
}
