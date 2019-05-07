#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

pid_t childpid;

int parse(char *in, char **out) {

    char *word;
    int pos = 0;
    int isBackground = 0;

    word = strtok(in, " \n\t\r");
    while (word != NULL) {
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

void sigint_handler() {
    if (childpid) {
        printf("Killing child PID: %i\n", childpid);
        kill(childpid, SIGINT);
        childpid = 0;
    }
    else {
        printf("\n");
        exit(0);
    }
}

void sigtstp_handler() {
    if (childpid) {
        printf("Stopping PID: %i\n", childpid);
        kill(childpid, SIGTSTP);
    }
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
        
        if (!strcmp(args[0], "logout")) break;

        /*if (!strcmp(args[0], "fg")) {
            kill(childpid, SIGCONT);
            
        }*/

        pid_t pid, wpid;
        int status = 0;

        signal(SIGINT, sigint_handler);
        signal(SIGTSTP, sigtstp_handler);

        pid = fork();
        childpid = pid;

        if (pid == 0) {
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

            if (setpgid(pid, pid) != 0) {
                perror("setpgid() error");
            }
            printf("parent group: %d\n", (int) getpgrp());

            wpid = waitpid(pid, &status, WUNTRACED);

            childpid = 0;
        }

        input = NULL;
        isBackground = 0;
        //args = NULL;
    }

    free(input);
    free(args);

    return 0;
}
