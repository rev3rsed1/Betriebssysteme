#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <termios.h>

pid_t childpid, parentpgid;
int status;
int childcount = 0;
struct termios tcattr;

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
    if (childpid && childpid == getpgrp()) {
        printf("Killing child PID: %i, Parentgid:%i\n", childpid, parentpgid);
        kill(childpid, SIGINT);
        childpid = 0;
    }
    else if (!childcount) {
        printf("\n");
        exit(0);
    }
}

void sigtstp_handler() {
    if (childpid && getpgrp() == childpid) {
        printf("Stopping PID: %i\n", childpid);

        tcgetattr(STDOUT_FILENO, &tcattr);
        kill(childpid, SIGTSTP);
    }
    else if (!childcount) {
        exit(0);
    }
}

void sigchld_handler() {
    printf("Caught SIGCHLD PID: %i\n", getpid());

    pid_t wpid;
    int status;
 
    if (getpgrp() == parentpgid) {
        //printf("WIFEXITED: %i", WIFEXITED(status));

        //while (!WIFEXITED(status)) {
        //    printf("waiting..\n");
        wpid = waitpid(childpid, &status, WNOHANG);
        //if (wpid <= 0) return; 
        //}

        childcount--;
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

        if (!strcmp(args[0], "fg")) {
            printf("Resuming PID to fg: %i\n", childpid); 
            kill(childpid, SIGCONT);

            waitpid(childpid, &status, WUNTRACED);

            //tcsetattr(STDOUT_FILENO, TCSADRAIN, &tcattr);

            continue;
        }

        if (!strcmp(args[0], "bg")) {
            printf("Resuming PID to bg: %i\n", childpid);
            kill(childpid, SIGCONT);

            if (setpgid(childpid, childpid) != 0) {
                perror("setpgid error");
            }

            continue;
        }

        pid_t pid, wpid;
        int status;

        signal(SIGINT, sigint_handler);
        signal(SIGTSTP, sigtstp_handler);
        signal(SIGCHLD, sigchld_handler);

        pid = fork();
        //childpid = pid;
        parentpgid = getpgrp();

        if (pid == 0) {
            sleep(1);
            printf("child group: %d\n", (int) getpgrp());

            //childcount++;

            if(execvp(args[0], args) == -1) {
                perror("Execute error");
            }
            return 0;
        }

        else if (isBackground) {
            childpid = pid;
            childcount++;

            if (setpgid(pid, pid) != 0) {
                perror("setpgid() error");
            }

            printf("[%i]\n", getpgrp());
            //sleep(1);
        }

        else {
            childpid = pid;
            childcount++;
            
            printf("parent group: %d\n", (int)getpgrp());

            wpid = waitpid(pid, &status, WUNTRACED);
        }

        input = NULL;
        isBackground = 0;
    }

    free(input);
    free(args);

    return 0;
}
