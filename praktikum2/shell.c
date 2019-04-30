#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

int parse(char *in, char **out) {

    char *word;
    int pos = 0;
    int isBackground = 0;

    word = strtok(in, " \n\t\r");
    while(word != NULL) {
        if(!strcmp(word, "&")){
            isBackground = 1;
            break;
        }

        out[pos] = word;
        pos++;

        word = strtok(NULL, " \n\t\r");
    }
    
    out[pos] = NULL;
    return isBackground;

}

int main() {

    char *input;
    char **args = malloc(sizeof(char*) * 64);
    int isBackground = 0;

    while(1) {
        printf(">>> ");
        
        ssize_t buffer = 0;
        getline(&input, &buffer, stdin);

        isBackground = parse(input, args);
        
        if(!strcmp(args[0], "logout")) return 0;

        pid_t pid;
        int status;

        pid = fork();

        if(pid == 0) {
            if(execvp(args[0], args) == -1) {
                perror("Execute error");
            }
            return 0;
        }
        else if (isBackground) {
            printf("[%i]\n", pid);
            sleep(1);
        }
        else {
            waitpid(pid,&status, WUNTRACED);
        }

        input = NULL;
        isBackground = 0;
        //args = NULL;
    }

    free(input);
    free(args);

    return 0;
}
