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
#include "utils.h"


#define PORT 28900
#define RESPONSE_COOLDOWN 900 // 15 minutes in seconds
#define BUFFER_SIZE 1024

// Last time a response was sent
time_t last_response_time;


/**
 * Checks if server is allowed to respond (every 15 min).
 *
 * @return 1 if the server is allowed to respond (cooldown period passed), 0 if not (within the cooldown period).
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
 * Responds to a valid client request with userID and Access Point.
 *
 * @param client_socket The socket descriptor for the client connection.
 */
void respond_to_client(int client_socket) {
    // variable to hold server response
    char response[BUFFER_SIZE]; 
    char ap_name[BUFFER_SIZE];
    int len = BUFFER_SIZE;

    // Get the access point name
    get_ap_name(ap_name, len);

    // format the message correctly
    snprintf(response, BUFFER_SIZE, "%s %s\n", USER_ID, ap_name);

    // Send the response
    ssize_t bytes_sent = send(client_socket, response, strlen(response), 0);

    // Error checking for sending
    if (bytes_sent == -1) {
        perror("send");
    }
}


/**
 * Handles a single client connection. It will call respond_to_client
 * if the received request is valid ("Who are you?").
 *
 * @param client_socket The socket descriptor for the client connection.
 */
void handle_connection(int client_socket) {
    printf("Server - Connection established with client\n");
    char buffer[BUFFER_SIZE];
    // Receive data from client
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        printf("Server - Error receiving data\n");
        close(client_socket);
        return;
    }
    buffer[bytes_received] = '\0';
    // Check if the received request is valid
    if (strncmp(buffer, "Who are you?", 12) == 0) {
        printf("Server - Received request: %s\n", buffer);
        respond_to_client(client_socket);  // Send valid response
    } else {
        printf("Server - Invalid request: %s\n", buffer);
    }
    // Close the client socket after responding
    close(client_socket);
}

/**
 * Sets up the TCP server socket to listen for incoming connections.
 *
 * @return The socket descriptor for the server, or -1 if an error occurred.
 */
int setup_server_socket() {
    int sockfd;
    // Create a TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        printf("Server - Socket creation failed\n");
        return -1;  // Return -1 if socket creation failed
    }

    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP
    server_addr.sin_port = htons(PORT);  // Set the server port

    // Bind the socket to the address and port
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Server - Bind failed\n");
        close(sockfd);
        return -1;
    }

    // Start listening for incoming connections
    if (listen(sockfd, 3) < 0) {
        perror("listen");
        close(sockfd);
        return -1; // Return -1 if listening fails
    }

    printf("Server - Server listening on port %d\n", PORT);

    return sockfd;  // Return the socket descriptor for the server
    
}


/**
 * Main server loop using select().
 * This loop continuously accepts new connections and processes them.
 */
void run_server() {
    int new_socket;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int server_socket = setup_server_socket();
    if (server_socket < 0) return; // Exit if server setup fails

    while(1) {
        // Accept a new client connection
        new_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
        if (new_socket < 0) {
            perror("accept");
            continue; // Try to accept new connections on failure
        }
        // If the server is allowed to respond (cooldown period passed)
        if (should_respond()) {
            printf("Server - Connection made!\n");
            handle_connection(new_socket); // Handle the client request
        } else {
            printf("Server - Response cooldown active. Ignoring request.\n");
            close(new_socket); // Close the connection without responding
        }
    }
}

int main() {
    run_server();
    return 0;
}
