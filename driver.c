#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

pid_t client_pid = 0;
pid_t server_pid = 0;

void handle_sigint(int sig) {
    printf("\nCaught Ctrl+C, killing client and server...\n");

    if (client_pid > 0) kill(client_pid, SIGTERM);
    if (server_pid > 0) kill(server_pid, SIGTERM);

    wait(NULL); // Wait for children to exit
    wait(NULL);

    exit(0);
}

int main() {
    signal(SIGINT, handle_sigint);

    client_pid = fork();
    if (client_pid == 0) {
        execl("./client", "client", NULL);
        perror("Failed to exec client");
        exit(1);
    }

    server_pid = fork();
    if (server_pid == 0) {
        execl("./server", "server", NULL);
        perror("Failed to exec server");
        exit(1);
    }

    // Parent: wait forever or until signal
    while (1) {
        pause();  // Sleep until a signal arrives
    }

    return 0;
}
