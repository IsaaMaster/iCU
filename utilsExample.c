#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Your user ID
const char* USER_ID = "isong";

/**
 * Retrieves the access point name or BSSID.
 */
void get_ap_name(char* ap_buffer, int len){
    FILE *fp;
    char path[512];
    char *bssid = NULL;

    // Run the Windows command from WSL
    fp = popen("powershell.exe -Command \"netsh wlan show interfaces\"", "r");
    if (fp == NULL) {
        perror("popen failed");
        return;
    }

    while (fgets(path, sizeof(path), fp) != NULL) {
        // Look for the line containing "BSSID"
        if (strstr(path, "BSSID") != NULL) {
            // Strip leading spaces and newline
            char *start = strchr(path, ':');
            if (start != NULL) {
                start++; // move past the colon
                while (*start == ' ') start++; // skip spaces
                bssid = strdup(start);
                if (bssid != NULL) {
                    // Remove trailing newline
                    bssid[strcspn(bssid, "\r\n")] = 0;
                }
            }
            break;
        }
    }

    pclose(fp);

    if (bssid) {
        strcpy(ap_buffer, bssid);
        printf("Connected BSSID: %s\n", bssid);
        free(bssid);
    } else {
        printf("BSSID not found.\n");
    }

}