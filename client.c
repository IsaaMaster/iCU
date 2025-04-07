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
int probe_host(const char* ip_address);

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
