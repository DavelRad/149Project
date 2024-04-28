#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <state_info>\n", argv[0]);
        return 1;
    }

    // Output the header for the report
    printf("****************************************************************\n");
    printf("The current system state is as follows:\n");
    printf("****************************************************************\n");

    // Output the state information passed to the reporter
    for (int i = 1; i < argc; i++) {
        printf("%s\n", argv[i]);
    }

    // Optionally, you can format the output more precisely based on your project's needs
    return 0;
}
