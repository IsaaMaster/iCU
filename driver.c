#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

pid_t client_pid = 0;
pid_t server_pid = 0;

/**
 * Signal handler for SIGINT (Ctrl+C).
 * This function is called when the user presses Ctrl+C, and it gracefully shuts down
 * both the client and the server by terminating their processes and waiting for them to exit.
 * It then exits the program.
 */
void handle_sigint(int sig) {
    printf("\nCaught Ctrl+C, killing client and server...\n");

    // Terminate client process if it exists
    if (client_pid > 0) kill(client_pid, SIGTERM);
    // Terminate server process if it exists
    if (server_pid > 0) kill(server_pid, SIGTERM);

    // Wait for both client and server processes to exit
    wait(NULL); // Wait for children to exit
    wait(NULL);

    exit(0);
}

int main() {
    // Set up signal handler for SIGINT (Ctrl+C)
    signal(SIGINT, handle_sigint);
    // Fork a new process for the client
    client_pid = fork();
    if (client_pid == 0) {
        // In the client process: execute the client program
        execl("./client", "client", NULL);
        perror("Failed to exec client");
        exit(1);
    }
    // Fork a new process for the server
    server_pid = fork();
    if (server_pid == 0) {
        // In the server process: execute the server program
        execl("./server", "server", NULL);
        // If execl() fails, print an error and exit
        perror("Failed to exec server");
        exit(1);
    }

    // Parent: wait forever or until signal
    while (1) {
        pause();  // Sleep until a signal arrives
    }

    return 0;
}
