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
#include <errno.h>
#include <regex.h>
#include "utils.h"


#define PORT 28900
#define BUFFER_SIZE 1024
#define SCAN_INTERVAL 60  // seconds between scans
#define TIMEOUT_SEC 5  // timeout for connecting to a server
#define MAX_IPS 1024  // max number of IPs to handle per scan

/**
 * Sends a simple HTTP Get request to the provided URL using libcurl
 *
 * @param url The full URL to send the GET request to
 */
void send_http_get_curl(const char* url) {
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);  // Set the URL to send the GET request to
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_perform(curl);  // Send the request
        curl_easy_cleanup(curl);
    }
}


/**
 * Parses the server's response and submits detection via HTTP GET.
 * Expected response format: "<their_userid> <access_point>"
 *
 * @param sockfd The socket file descriptor connected to the server
 */
void handle_response(int sockfd){
    // setup string to receive response to "Who are you?
    char buffer[BUFFER_SIZE];

    // receive the response and store it in buffer
    int bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
    if (bytes_received <= 0) {
        printf("Client - Error receiving data\n");
        close(sockfd);
        return;
    }
    buffer[bytes_received] = '\0';  // Null-terminate the string

    // split the string into userid and access point
    char their_userid[BUFFER_SIZE];
    char ap_name[BUFFER_SIZE];

    // Parse the response string
    sscanf(buffer, "%s %s\n", their_userid, ap_name);

    printf("Client - Received response: %s %s\n", their_userid, ap_name);

    // Construct the URL, and make the HTTP GET Request
    char url[BUFFER_SIZE];
    snprintf(url, sizeof(url),
        "http://vmwardrobe.westmont.edu:28900?i=%s&u=%s&where=%s",
        USER_ID, their_userid, ap_name);
    send_http_get_curl(url);
}
/**
 * Attempts to connect to a given IP on prt 28900 and sends a probe
 * If successful, returns the socket descriptor.
 *
 * @param ip_address The IP address to connect to (as a string)
 * @return The socket file descriptor if successful, or -1 if the connection fails.
 */
int probe_host(const char* ip_address) {
    printf("Client - Probing IP: %s\n", ip_address);

    char send_buffer[BUFFER_SIZE];
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);  // Create TCP socket

    if (sockfd < 0) {
        printf("Client - Socket creation failed\n");
        return -1;
    }

    // Set socket to non-blocking
    fcntl(sockfd, F_SETFL, O_NONBLOCK);

    // Setup server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(ip_address);

    // Attempt to connect
    int connect_result = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (connect_result < 0) {
        if (errno != EINPROGRESS) {
            // Immediate failure
            printf("Client - Immediate connect failed\n");
            close(sockfd);
            return -1;
        }

        // Wait for the socket to be writable (indicating connection success/failure)
        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(sockfd, &writefds);
        struct timeval timeout;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

        int select_result = select(sockfd + 1, NULL, &writefds, NULL, &timeout);
        if (select_result <= 0) {
            printf("Client - Connection timeout or select error\n");
            close(sockfd);
            return -1;
        }

        // Check if the socket has any errors
        int so_error;
        socklen_t len = sizeof(so_error);
        getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
        if (so_error != 0) {
            printf("Client - Connection failed with error: %d\n", so_error);
            close(sockfd);
            return -1;
        }
    }

    // Restore socket to blocking mode (optional)
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) & ~O_NONBLOCK);

    // Send initial probe message
    printf("Client - Connected to server at IP: %s\n", ip_address);
    send(sockfd, "Who are you?", strlen("Who are you?"), 0);
    printf("Client - Sent request to server\n");

    return sockfd;
}



/**
 * Scans the local network for open TCP port 28900.
 * Uses masscan to quickly scan and regex to extract IPs.
 * For each responsive host, sends a probe and handles the server response.
 */
void scan_network() {
    char buffer[BUFFER_SIZE];
    FILE *fp;
    char path[1024];
    char *ips[MAX_IPS];
    int ip_count = 0;
    char last_ip[16] = ""; // To store the last probed IP


    // Use masscan command for faster scanning
    const char *masscan_command = "timeout 7s masscan 10.124.0.0-10.124.10.255 -p28900 --open --rate=2000 --wait=1 --max-rate=2000";

    printf("Client: Scanning network for open port 28900...\n");

    // Open a pipe to run the Masscan command
    fp = popen(masscan_command, "r");
    if (fp == NULL) {
        perror("Failed to run Masscan");
        return;
    }

    // Use regex to match lines like "Discovered open port 28900/tcp on 10.124.137.142"
    regex_t regex;
    regcomp(&regex, "Discovered open port 28900/tcp on ([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+)", REG_EXTENDED);

    while (fgets(path, sizeof(path), fp) != NULL) {
        regmatch_t matches[2];
        if (regexec(&regex, path, 2, matches, 0) == 0) {
            int len = matches[1].rm_eo - matches[1].rm_so;
            if (ip_count < MAX_IPS) {
                ips[ip_count] = strndup(path + matches[1].rm_so, len);
                ip_count++;
            }
        }
    }

    regfree(&regex);
    pclose(fp);

    // Now loop through the discovered IPs
    printf("Client: Found %d host(s) with port 28900 open:\n", ip_count);
    for (int i = 0; i < ip_count; i++) {
        // Avoid sending duplicate probe to the same IP
        if (strcmp(ips[i], last_ip) == 0) {
            printf("Client - Skipping IP %s (same as previous)\n", ips[i]);
            continue; // Skip this IP and continue to the next one
        }

        printf(" - %s\n", ips[i]);
        int sockfd = probe_host(ips[i]);  // Try connecting
        if (sockfd >= 0) {
            printf("Client - Found server at IP: %s\n", ips[i]);
            handle_response(sockfd);  // Handle response if connected
        } else {
            printf("Client - No response received from server at IP: %s\n", ips[i]);
        }
        close(sockfd);
        free(ips[i]); // Free the allocated memory

        strcpy(last_ip, ips[i]);
    }

    return;
}

/**
 * Sends an uptime heartbeat to vmwardrobe when called.
 *
 * @param seconds_alive The number of seconds this client has been running.
 */
void send_uptime(int seconds_alive) {
    // Construct heartbeat URL
    char url[BUFFER_SIZE];
    snprintf(url, sizeof(url), "http://vmwardrobe.com/heartbeat?user=%s&uptime=%d", USER_ID, seconds_alive);

    // Send GET request
    send_http_get_curl(url);
    return; 
};

/**
 * Main client loop.
 * Repeatedly scans the network and sends an uptime heartbeat once per minute.
 */
void run_client() {
    int seconds_alive = 0;
    time_t last_time = time(NULL);

    while(1){
        // Scan continuously for 60 seconds
        while (last_time - time(NULL) < 60) {
            scan_network();
        }
        printf("Client - Sending Uptime...");

        // Update uptime and send it
        seconds_alive += last_time - time(NULL);
        last_time = time(NULL);
        send_uptime(seconds_alive);
    }
};

int main() {
    run_client();
    return 0;
}
