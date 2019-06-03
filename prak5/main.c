#include <stdio.h>
#include <stdlib.h>

int main () {
    printf("Starting!\n");

    int t = 0;

    int* mem;
    //if (!(mem = malloc(sizeof(int) * 1024))) {
      //printf("Unable to allocate!\n");
      //return 0;
    //}

    while (1) {
      mem = (int*) calloc(1, sizeof(char)*1024*1024);
      if (!mem) {
        printf("Maximum memory allocated: %i!\n", t);
        break;
      }
      t++;
    }
    while (1) {}

    return 0;
}
