#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

// Function Prototypes
void process_command(char *command);
void handle_time_unit();
void unblock_process();
void print_system_state();
void print_turnaround_time_and_terminate();

int main() {
    char buffer[BUFFER_SIZE];

    // Read commands from stdin (piped from the commander)
    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        // Remove newline character if present
        buffer[strcspn(buffer, "\n")] = 0;

        // Process the command
        process_command(buffer);
    }

    return 0;
}

// Process the received command
void process_command(char *command) {
    if (strcmp(command, "Q") == 0) {
        handle_time_unit();
    } else if (strcmp(command, "U") == 0) {
        unblock_process();
    } else if (strcmp(command, "P") == 0) {
        print_system_state();
    } else if (strcmp(command, "T") == 0) {
        print_turnaround_time_and_terminate();
    } else {
        fprintf(stderr, "Unknown command received: %s\n", command);
    }
}

void handle_time_unit() {
    // Handle one unit of time passing
    printf("Handling time unit...\n");
}

void unblock_process() {
    // Handle unblocking a process
    printf("Unblocking process...\n");
}

void print_system_state() {
    // Print the current state of the system
    printf("System State:\n");
    // Placeholder for actual state printing
}

void print_turnaround_time_and_terminate() {
    // Calculate and print average turnaround time
    printf("Average Turnaround Time:\n");
    // Placeholder for actual calculation and printing

    // Terminate the simulation
    exit(0);
}
