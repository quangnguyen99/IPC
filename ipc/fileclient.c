#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

const int port = 8080;
const int bsize = 1024;
const int lineSize = 1024;
const int fileSize = 10000;

void cleanup(char **lines, int numLines, FILE *file);
void editFile(char *filename);

int main(int argc, char *argv[]) {
    int sock = 0;
    struct sockaddr_in servAddr;
    char buffer[bsize];
    char filename[] = "text.txt"; // I use fixed file name for this program
    ssize_t bytesReceived;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr);

    if (connect(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    write(sock, filename, strlen(filename));

    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open file");
        close(sock);
        return 1;
    }

    printf("Receiving file...\n");
    while ((bytesReceived = read(sock, buffer, bsize - 1)) > 0) {
        fwrite(buffer, 1, bytesReceived, file);
        if (bytesReceived < bsize - 1) {
            break;
        }
    }
    fclose(file);

    printf("File received and saved as '%s'.\n", filename);

    editFile(filename);

    file = fopen(filename, "r");
    if (file) {
        while ((bytesReceived = fread(buffer, 1, bsize, file)) > 0) {
            write(sock, buffer, bytesReceived);
        }
        fclose(file);
        printf("Updated file sent to server.\n");
    }

    close(sock);
    return 0;
}

// Edit file
void editFile(char *filename) {
    char **lines = malloc(fileSize * sizeof(char *));
    int numLines = 0;

    FILE *ourFile = fopen(filename, "r+");
    if (!ourFile) {
        perror("Error opening file for editing");
        return;
    }

    lines[0] = malloc(lineSize * sizeof(char));
    while (fgets(lines[numLines], lineSize, ourFile)) {
        numLines++;
        lines[numLines] = malloc(lineSize * sizeof(char));
    }
    free(lines[numLines]);

    int option = -1;
    while (option != 0) {
        printf("\n1: Edit a line\n");
        printf("2: Delete a line\n");
        printf("3: Insert a line\n");
        printf("4: Show file\n");
        printf("0: Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &option);

        int line = 0;
        switch (option) {
            case 1:
                printf("Enter line to edit: ");
                scanf("%d", &line);
                if (line > 0 && line <= numLines) {
                    printf("Editing line %d: ", line);
                    getchar();
                    fgets(lines[line - 1], lineSize, stdin);
                    size_t len = strlen(lines[line - 1]);
                    if (len > 0 && lines[line - 1][len - 1] != '\n') {
                        lines[line - 1][len] = '\n';
                        lines[line - 1][len + 1] = '\0';
                    }
                } else {
                    printf("Invalid line number.\n");
                }
                break;
            case 2:
                printf("Enter line to delete: ");
                scanf("%d", &line);
                if (line > 0 && line <= numLines) {
                    free(lines[line - 1]);
                    for (int i = line - 1; i < numLines - 1; i++) {
                        lines[i] = lines[i + 1];
                    }
                    numLines--;
                } else {
                    printf("Invalid line number.\n");
                }
                break;
            case 3:
                printf("Enter line to insert at: ");
                scanf("%d", &line);
                if (line > 0 && line <= numLines + 1) {
                    char *newLine = malloc(lineSize);
                    printf("Enter new line content: ");
                    getchar();
                    fgets(newLine, lineSize, stdin);
                    size_t len = strlen(newLine);
                    if (len > 0 && newLine[len - 1] != '\n') {
                        newLine[len] = '\n';
                        newLine[len + 1] = '\0';
                    }
                    for (int i = numLines; i >= line; i--) {
                        lines[i] = lines[i - 1];
                    }
                    lines[line - 1] = newLine;
                    numLines++;
                } else {
                    printf("Invalid line number.\n");
                }
                break;
            case 4:
                printf("\nSHOW FILE\n");
                for (int i = 0; i < numLines; i++) {
                    printf("%d: %s", i + 1, lines[i]);
                }
                printf("END FILE\n");
                break;
            case 0:
                cleanup(lines, numLines, ourFile);
                return;
            default:
                printf("Invalid option. Try again.\n");
        }
    }
}

//Cleanup
void cleanup(char **lines, int numLines, FILE *file) {
    fseek(file, 0, SEEK_SET);

    for (int i = 0; i < numLines; i++) {
        fprintf(file, "%s", lines[i]);
        free(lines[i]);
    }

    long newFileSize = ftell(file);
    fflush(file);
    ftruncate(fileno(file), newFileSize);

    fclose(file);
    free(lines);
}
