#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024

int main() {
    int fd[2]; // File descriptors for the pipe
    pid_t pid;
    char buffer[BUFFER_SIZE];

    // Create a pipe
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork to create the process manager process
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process: Process Manager
        close(fd[1]); // Close unused write end of the pipe

        // Execute process manager program
        // Assuming process_manager will read from fd[0]
        dup2(fd[0], STDIN_FILENO); // Redirect standard input to read from the pipe
        close(fd[0]); // Close the original file descriptor after duplicating

        execlp("./process_manager", "process_manager", (char *)NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process: Commander
        close(fd[0]); // Close unused read end of the pipe

        // Read commands from standard input
        while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
            // Send the command to the process manager
            if (write(fd[1], buffer, strlen(buffer)) == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }

            // Optionally, add control to end loop if needed
            if (strncmp(buffer, "T", 1) == 0) {
                break; // 'T' command is the termination command
            }
        }

        // Close the write end of the pipe to signal no more data
        close(fd[1]);

        // Wait for the child process to complete
        wait(NULL);
    }

    return 0;
}
