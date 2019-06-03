#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

pid_t childpid, parentpgid, stoppedchild, childpgid;
int status;
int childcount = 0;


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
    printf("Caught SIGINT\n");
    if (childpid != 0 && childpgid == getpgrp()) {
        printf("Killing child PID: %i, Parentgid:%i\n", childpid, parentpgid);
        kill(childpid, SIGINT);
        //childpid = 0;
    }
    else if (!childcount) {
        char *input;
        ssize_t buffer = 0;

        printf("Sure? (y/n)");
        getline(&input, &buffer, stdin);
        if (input[0] == 'y') exit(0);
        
    }
}

void sigtstp_handler() {
    if (childpid && getpgrp() == childgpid) {
        printf("Stopping PID: %i\n", childpid);

        //tcgetattr(STDOUT_FILENO, &tcattr);
        kill(childpid, SIGTSTP);
        stoppedchild = childpid;
    }
    else if (!childcount) {
        exit(0);
    }
}

void sigchld_handler() {
    printf("Caught SIGCHLD PID: %i\n", getpid());

    pid_t wpid;
    int status;
 
    if (childpid && getpgrp() == parentpgid) {
        wpid = waitpid(childpid, &status, WNOHANG);
        
        printf("Status: %i\n", wpid);

        if (wpid != 0) childcount--;
    }
}

int main() {
    
    char *input;
    char **args = malloc(sizeof(char*) * 64);
    int isBackground = 0;

    while(1) {
        signal(SIGINT, sigint_handler);
        signal(SIGTSTP, sigtstp_handler);
        signal(SIGCHLD, sigchld_handler);

        printf(">>> ");
        
        ssize_t buffer = 0;
        getline(&input, &buffer, stdin);

        isBackground = parse(input, args);
        
        if (!strcmp(args[0], "count")) {
            printf("Count: %i\n", childcount);
            continue;
        }

        if (!strcmp(args[0], "logout")) {
            if (!childcount) {
                printf("Sure? (y/n)");
                getline(&input, &buffer, stdin);
                if (input[0] == 'y') break;
                else continue;
            }
            else if (childcount) {
                printf("Childs left...\n");
                continue;
            }
        }

        if (!strcmp(args[0], "fg")) {
            childpid = stoppedchild;

            printf("Resuming PID to fg: %i\n", childpid); 
            kill(childpid, SIGCONT);

            waitpid(childpid, &status, WUNTRACED);

            //tcsetattr(STDOUT_FILENO, TCSADRAIN, &tcattr);

            continue;
        }

        if (!strcmp(args[0], "bg")) {
            printf("Resuming PID to bg: %i\n", childpid);
            pid_t temppid = childpid;
            childpid = 0;
            kill(temppid, SIGCONT);

            //if (setpgid(getpid(), getpid()+1) != 0) {
              //  perror("setpgid error");
            //}
            //signal(SIGINT, sigint_handler);
            childpid = 0;
            //parentpgid++;

            continue;
        }

        pid_t pid, wpid;
        int status;


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
            //parentpgid = getpgrp();
            childcount++;
            childpgid = getpgrp();


            printf("[%i]\n", getpgrp());
            //sleep(1);
        }

        else {
            childpid = pid;
            //parentpgid = getpgrp();
            childcount++;
            childpgid = pid;

            if (setpgid(pid, pid) != 0) {
                perror("setpgid() error");
            }

            
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
