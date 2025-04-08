// icu_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <time.h>

#define PORT 28900
#define BUFFER_SIZE 1024
#define SCAN_INTERVAL 60  // seconds between scans

// Your user ID
const char* USER_ID = "your_userid_here";

/**
 * Scans the local network for open TCP port 28900.
 */
void scan_network();

/**
 * Attempts to connect to a given IP and send "Who are you?".
 */
int probe_host(const char* ip_address) {
    char send_buffer[BUFFER_SIZE];
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(ip_address);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection creation failed");
        return -1;
    }
    memset(&server_addr, 0, sizeof(server_addr));

    send(sockfd, "Who are you?", strlen("Who are you?"), 0);
    
    return sockfd; 
}

/**
 * Parses the server's response and submits detection via HTTP GET.
 */
void handle_response(const char* ip, const char* response);

/**
 * Sends an uptime heartbeat to vmwardrobe once a minute.
 */
void send_uptime(int seconds_alive);

/**
 * Main client loop.
 */
void run_client();

int main() {
    run_client();
    return 0;
}
