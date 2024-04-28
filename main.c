#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    // Fork to create the commander process
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process: Execute the commander program
        execlp("./commander", "commander", (char *)NULL);
        perror("execlp");  // If execlp returns, there was an error
        exit(EXIT_FAILURE);
    } else {
        // Parent process: Wait for the commander process to finish
        wait(NULL);
    }

    printf("Simulation has ended.\n");
    return 0;
}
