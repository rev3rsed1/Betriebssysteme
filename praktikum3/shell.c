#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

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

void sigint_handler(pid_t pid) {
    printf("Killing child PID: %i\n", pid);
    kill(pid, SIGKILL);
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

        pid_t pid, wpid;
        int status = 0;

        //signal(SIGINT, sigint_handler);

        pid = fork();

        if(pid == 0) {
            //sleep(1);
            printf("child group: %d\n", (int) getpgrp());

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
            printf("parent group: %d\n", (int) getpgrp());

            if (setpgid(pid, 0) != 0) {
                perror("setpgid() error");
            }

            //if (!tcsetpgrp(STDIN_FILENO, pid)) {
                //perror("tcsetpgrp() error");
            //}
            //signal(SIGINT, sigint_handler(pid));

            //while (!WIFEXITED(status) && !WIFSIGNALED(status)){}
            wpid = waitpid(pid, &status, WUNTRACED);
        }

        input = NULL;
        isBackground = 0;
        //args = NULL;
    }

    free(input);
    free(args);

    return 0;
}
