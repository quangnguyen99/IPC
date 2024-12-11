#include <stdio.h>
#include <stdlib.h>

const int lineSize = 1024;
const int fileSize = 10000; // our current limit

void printMenu(){
    printf("\n1: Edit a line\n");
    printf("2: delete a line\n");
    printf("3: insert a line\n");
    printf("4: show file\n");
    printf("0: exit\n");
}

void editLine(char* line){
    printf("Editing line %s to: ", line);
    char c;
    while ((c = getchar()) != '\n' && c != EOF);
    fgets(line, lineSize, stdin);
}

void delLine(int line, char** ls, int* nL){
    char* oldPointer = ls[line];
    for(int i = line; i < *nL-1; i++){
        ls[i] = ls[i+1];
    }
    free(oldPointer);
    (*nL)--;
}

void insLine(int line, char** ls, int* nL){
    char* newLine = malloc(lineSize);
    char c;

    printf("New text to insert at line %d:\n", line);

    while ((c = getchar()) != '\n' && c != EOF);
    fgets(newLine, lineSize, stdin);

    for(int i = *nL; i > line; i--){
        ls[i] = ls[i-1];
    }

    ls[line] = newLine;
    (*nL)++;
}

void cleanup(char** ls, int nL, FILE* f){
    //write to the file
    fseek(f, 0, SEEK_SET);
    for(int i = 0; i < nL; i++){
        fputs(ls[i], f);
        free(ls[i]);
    }

	free(ls);
	fclose(f);
}

void printFile(char** ls, int nL){
    printf("\nSHOW FILE\n");
    for(int i = 0; i < nL; i++){
        printf("%d: %s", i, ls[i]);
    }
    printf("END FILE\n");
}

int main(int argc, char* argv[]){

	FILE* ourFile = fopen(argv[1], "r+");
	char** lines = malloc(fileSize * sizeof(char*));
	int numLines = 0;
	
	lines[0] = malloc(lineSize * sizeof(char));
	
	while(fgets(lines[numLines],lineSize,ourFile)){
		numLines++;
		lines[numLines] = malloc(lineSize * sizeof(char));
	}

	free(lines[numLines]);

    int option = -1;
    while(option != 0){
        printMenu();
        scanf("%d", &option);
        int line = 0;
        switch(option){
            case 1:
                // Call edit function
                printf("Enter a line to edit: ");
                scanf("%d", &line);
                editLine(lines[line]);
                break;
            case 2:
                // Call delete function
                printf("Enter a line to delete: ");
                scanf("%d", &line);
                delLine(line, lines, &numLines);
                break;
            case 3:
                // Call insert function
                printf("Enter a line to insert at: ");
                scanf("%d", &line);
                insLine(line, lines, &numLines);
                break;
            case 4:
                printFile(lines, numLines);
                break;
            case 0:
                cleanup(lines, numLines, ourFile);
                return 0;
            default:
                printf("No idea what that meant!\n");
        }
    }
	
	return 0;
}
