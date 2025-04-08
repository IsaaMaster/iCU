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
void handle_response(int sockfd, const char* ip, const char* response);
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    int bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        perror("Error receiving data");
        close(sockfd);
        return;
    }

    buffer[bytes_received] = '\0';

    char server_userid[BUFFER_SIZE];
    char ap_name[BUFFER_SIZE];

    if (sscanf(buffer, "%s %s", server_userid, ap_name) != 2) {
        fprintf(stderr, "Invalid response format: %s\n", buffer);
        close(sockfd);
        return;
    }

    int report_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (report_sock < 0) {
        perror("Socket creation failed for reporting");
        close(sockfd);
        return;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(28900);
    server_addr.sin_addr.s_addr = inet_addr("vmwardrobe.westmont.edu");

    if (connect(report_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to vmwardrobe failed");
        close(sockfd);
        close(report_sock);
        return;
    }



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
