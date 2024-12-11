#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BSIZE 1024
#define LINESIZE 1024
#define FILESIZE 10000

void printMenu() {
    printf("\n1: Edit a line\n");
    printf("2: Delete a line\n");
    printf("3: Insert a line\n");
    printf("4: Show file\n");
    printf("0: Exit\n");
}

void editLine(char *line) {
    printf("Editing line %s to: ", line);
    char c;
    while ((c = getchar()) != '\n' && c != EOF);
    fgets(line, LINESIZE, stdin);
}

void delLine(int line, char **ls, int *nL) {
    char *oldPointer = ls[line];
    for (int i = line; i < *nL - 1; i++) {
        ls[i] = ls[i + 1];
    }
    free(oldPointer);
    (*nL)--;
}

void insLine(int line, char **ls, int *nL) {
    char *newLine = malloc(LINESIZE);
    char c;

    printf("New text to insert at line %d:\n", line);

    while ((c = getchar()) != '\n' && c != EOF);
    fgets(newLine, LINESIZE, stdin);

    for (int i = *nL; i > line; i--) {
        ls[i] = ls[i - 1];
    }

    ls[line] = newLine;
    (*nL)++;
}

void cleanup(char **ls, int nL, int clientSocket) {
    char buffer[BSIZE];
    for (int i = 0; i < nL; i++) {
        write(clientSocket, ls[i], strlen(ls[i]));
        free(ls[i]);
    }
    free(ls);
}

void handleEditing(int clientSocket) {
    char buffer[BSIZE];
    ssize_t bytesReceived;

    char **lines = malloc(FILESIZE * sizeof(char *));
    int numLines = 0;

    lines[0] = malloc(LINESIZE * sizeof(char));

    printf("Receiving file contents from server...\n");
    while ((bytesReceived = read(clientSocket, buffer, BSIZE - 1)) > 0) {
        buffer[bytesReceived] = '\0';
        char *line = strtok(buffer, "\n");
        while (line != NULL) {
            lines[numLines] = malloc(LINESIZE * sizeof(char));
            strncpy(lines[numLines], line, LINESIZE);
            numLines++;
            line = strtok(NULL, "\n");
        }
    }

    printf("File received successfully.\n");

    int option = -1;
    while (option != 0) {
        printMenu();
        scanf("%d", &option);
        int line = 0;
        switch (option) {
            case 1:
                printf("Enter a line to edit: ");
                scanf("%d", &line);
                if (line >= 0 && line < numLines) {
                    editLine(lines[line]);
                } else {
                    printf("Invalid line number.\n");
                }
                break;
            case 2:
                printf("Enter a line to delete: ");
                scanf("%d", &line);
                if (line >= 0 && line < numLines) {
                    delLine(line, lines, &numLines);
                } else {
                    printf("Invalid line number.\n");
                }
                break;
            case 3:
                printf("Enter a line to insert at: ");
                scanf("%d", &line);
                if (line >= 0 && line <= numLines) {
                    insLine(line, lines, &numLines);
                } else {
                    printf("Invalid line number.\n");
                }
                break;
            case 4:
                for (int i = 0; i < numLines; i++) {
                    printf("%d: %s", i, lines[i]);
                }
                break;
            case 0:
                cleanup(lines, numLines, clientSocket);
                printf("File saved and sent to server.\n");
                return;
            default:
                printf("Invalid option!\n");
        }
    }
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char filename[256];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    printf("Enter the filename to request: ");
    scanf("%s", filename);

    send(sock, filename, strlen(filename), 0);
    printf("File requested: %s\n", filename);

    handleEditing(sock);

    close(sock);
    return 0;
}
