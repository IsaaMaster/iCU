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
#include <curl/curl.h>


#define PORT 28900
#define BUFFER_SIZE 1024
#define SCAN_INTERVAL 60  // seconds between scans

// Your user ID
const char* USER_ID = "Lynx";


/**
 * Scans the local network for open TCP port 28900.
 */
void scan_network() {
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

void send_http_get_curl(const char* url) {
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
}


/**
 * Parses the server's response and submits detection via HTTP GET.
 */
void handle_response(int sockfd){
    // setup string to receive response to "Who are you?
    char buffer[BUFFER_SIZE];

    // receive the response and store it in buffer
    int bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        perror("Error receiving data");
        close(sockfd);
        return;
    }
    buffer[bytes_received] = '\0';

    // split the string into userid and access point
    char their_userid[BUFFER_SIZE];
    char ap_name[BUFFER_SIZE];

    // Parse the response string
    sscanf(buffer, "%s %s\n", their_userid, ap_name);

    // Construct the URL, and make the HTTP GET Request
    char url[BUFFER_SIZE];
    snprintf(url, sizeof(url),
        "http://vmwardrobe.westmont.edu:28900?i=%s&u=%s&where=%s",
        USER_ID, their_userid, ap_name);
    send_http_get_curl(url);
}

/**
 * Sends an uptime heartbeat to vmwardrobe when called.
 */
void send_uptime(int seconds_alive) {
    char url[BUFFER_SIZE];
    snprintf(url, sizeof(url), "http://vmwardrobe.com/heartbeat?user=%s&uptime=%d", USER_ID, seconds_alive);
    send_http_get_curl(url);
    return; 
};
/**
 * Main client loop.
 */
void run_client() {
    return; 
};

int main() {
    run_client();
    return 0;
}
