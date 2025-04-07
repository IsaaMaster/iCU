// icu_server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>

#define PORT 28900
#define RESPONSE_COOLDOWN 900 // 15 minutes in seconds
#define BUFFER_SIZE 1024

// Your user ID
const char* USER_ID = "your_userid_here";

// Last time a response was sent
time_t last_response_time;

/**
 * Retrieves the access point name or BSSID.
 */
void get_ap_name(char* ap_buffer, size_t len);

/**
 * Checks if server is allowed to respond (every 15 min).
 */
int should_respond();

/**
 * Responds to a valid client request with userID and AP.
 */
void respond_to_client(int client_socket);

/**
 * Handles a single client connection.
 */
void handle_connection(int client_socket);

/**
 * Sets up the TCP server socket.
 */
int setup_server_socket();

/**
 * Main server loop using select().
 */
void run_server();

int main() {
    run_server();
    return 0;
}
