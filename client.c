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
void scan_network(){
    const char* base_ip_address = "10.124.";
    char ip_address[INET_ADDRSTRLEN]; // Buffer to hold the IP address string
    char buffer[BUFFER_SIZE];
    
    // Loop through IP range 10.124.0.1 to 10.124.63.254
    for (int i = 1; i <= 16383; i++) {
        int third_octet = (i - 1) / 254;  // Determine third octet based on current iteration
        int fourth_octet = (i - 1) % 254 + 1;  // Determine fourth octet, making sure it's in range 1-254

        snprintf(ip_address, sizeof(ip_address), "%s%d.%d", base_ip_address, third_octet, fourth_octet);

        // Call probe_host to check this IP
        int sockfd = probe_host(ip_address);

        if (sockfd >= 0) {
            printf("Found server at IP: %s\n", ip_address);
            
            // clear the buffer before use
            memset(buffer, 0, sizeof(buffer))

            // attempt to receive data from server
            int bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        
            // if data is received successfully, handle the response
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0'; // Null-terminate the response
                handle_response(ip_address, buffer);
            } else {
                printf("No response received from server at IP: %s\n", ip_address);
            }
            
            close(sockfd);
        }  else {
            printf("No server at IP: %s\n", ip_address);
        }
    }
}

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
