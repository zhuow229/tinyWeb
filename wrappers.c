#include "inc.h"

int Fork() {
    pid_t pid;
    if((pid = fork()) < 0) {
        fprintf(stderr, "fork error\n");
        exit(1);
    }
    return pid;
}