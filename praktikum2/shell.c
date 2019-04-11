#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

void parse(char *in, char **out) {

    char *word;
    int pos = 0;

    word = strtok(in, " \n\t\r");
    while(word != NULL) {
        out[pos] = word;
        pos++;

        word = strtok(NULL, " \n\t\r");
    }
    
    out[pos] = NULL;
}

int main() {

    char *input;
    char **args = malloc(sizeof(char*) * 64);

    while(1) {
        printf(">>> ");
        
        ssize_t buffer = 0;
        getline(&input, &buffer, stdin);

        parse(input, args);
        //printf("%i", strcmp(args[0],"logout"));
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
        else {
            //while(!WIFEXITED(status) && !WIFSIGNALED(status)) {
              //  waitpid(pid, &status, WUNTRACED | WCONTINUED);
            //}
            wait(&status);
        }

        input = NULL;
        //args = NULL;
    }

    free(input);
    free(args);

    return 0;
}
