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
const char* USER_ID = "lynx";

// Last time a response was sent
time_t last_response_time;

/**
 * Retrieves the access point name or BSSID.
 */
void get_ap_name(char* ap_buffer, size_t len);

/**
 * Checks if server is allowed to respond (every 15 min).
 */
int should_respond(){
    time_t now = time(NULL);
    if (difftime(now, last_response_time) >= RESPONSE_COOLDOWN) {
        last_response_time = now;
        return 1;
    }
    return 0;
}

/**
 * Responds to a valid client request with userID and AP.
 */
void respond_to_client(int client_socket, const char* access_point_name) {
    // variable to hold server response
    char response[BUFFER_SIZE];

    // format the message correctly
    snprintf(response, BUFFER_SIZE, "%s %s\n", USER_ID, access_point_name);

    // Send the response
    ssize_t bytes_sent = send(client_socket, response, strlen(response), 0);

    // Error checking for sending
    if (bytes_sent == -1) {
        perror("send");
    }
}


/**
 * Handles a single client connection. Will call respond_to_client if there is a valid request. 
 */
void handle_connection(int client_socket) {
    printf("Connection established with client\n");
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        perror("Error receiving data");
        close(client_socket);
        return;
    }
    buffer[bytes_received] = '\0';

    printf("Received request: %s\n", buffer);
    if (strncmp(buffer, "Who are you?", 12) == 0) {
        printf("Received request: %s\n", buffer);
        respond_to_client(client_socket);
    } else {
        printf("Invalid request: %s\n", buffer);
    }
    
    close(client_socket);
}

/**
 * Sets up the TCP server socket.
 */
int setup_server_socket() {
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        printf("Socket creation failed");
        return -1;
    }

    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Bind failed");
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 3) < 0) {
        perror("listen");
        close(sockfd);
        return -1; 
    }

    printf("Server listening on port %d\n", PORT);

    return sockfd;
    
}

/**
 * Main server loop using select().
 */
void run_server() {
    int new_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int server_socket = setup_server_socket();
    if (server_socket < 0) return;   

    while(1) {
        new_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (new_socket < 0) {
            perror("accept");
            continue;
        }

        if (should_respond()) {
            printf("connection made!\n");
            handle_connection(new_socket);
        } else {
            printf("Response cooldown active. Ignoring request.\n");
            close(new_socket);
        }
    }
}

int main() {
    run_server();
    return 0;
}
