#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#define DEBUG 0
#define PORT 9756
#define SUCC "Successfully received a number from Client\0"
#define FAIL "Failed to receive a valid number from Client\0"

int validate(char *msg) {
    char *template = "LikeServer0 00\0";
    int ii = 0;
    // check to make sure that the string is as long as the template
    if (strlen(msg) == strlen(template)) {
        for (ii = 0; ii < strlen(template); ii++) {
            // verify that the message is prefixed with LikeServer
            if (ii < 10) {
                if (msg[ii] != template[ii]) {
                    return -1;
                }
            }
            // verify that the message has an integer at position 10, 12, and 13
            if (ii == 10 || ii > 11) {
                if (msg[ii] > '9' || msg[ii] < '0') {
                    return -1;
                }
            }
            // verify that there is a space between position 10 and 12
            if (ii == 11) {
                if (msg[ii] != ' '){
                    return -1;
                }
            }
        }
    } else {
        return -1;
    }
    // return 1 if msg is valid
    return 1;
}

int extractValue(char *msg) {
    //extract value after 12 characters
    return atoi(msg+12);
}

int main(int argc, char **argv) {
    int sum = 0;
    #ifdef DEBUG
    printf("Hello from the primary like server\n");
    #endif
    // declare the variables for socket programming
    int sfd, cfd, valread;
    int opt = 1;
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    char buffer[1024] = { 0 };
    struct sockaddr_in address = { 0 };
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    socklen_t server_info_len = sizeof(address);
    time_t start_t, end_t;
    time(&start_t);
    time(&end_t);

    // Initialize the socket using IPv4 and TCP
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("Socket Failed to initialize");
        exit(1);
    }

    // set socket options
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("Set Socket Error");
        exit(1);
    }

    if (setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("Set Socket Error");
        exit(1);
    }

    // bind on the 9765 port number
    if (bind(sfd, (struct sockaddr*) &address, server_info_len) < 0) {
        perror("Error binding to port");
        exit(1);
    }

    // start listening on the socket
    if (listen(sfd, 0) < 0) {
        perror("Error while starting to listen");
        exit(1);
    }

    while (difftime(end_t, start_t) <= 340) {
        char buffer[1024] = { 0 };
        // accept the connection and keep the socket info for the client to send a response, has a timeout set for 10 seconds
        cfd = accept(sfd, (struct sockaddr*) &address, (socklen_t*)&server_info_len);

        // read the buffer then print to log and reply with the hello message
        valread = read(cfd, buffer, 1024);
        if (validate(buffer) > 0) {
            int likes = 0;
            likes = extractValue(buffer);
            #ifdef DEBUG
            if (likes) {
                printf("%d\n", likes);
            }
            #endif
            sum += likes;
            #ifdef DEBUG
            printf("%d\n", sum);
            #endif
            // print out the total and the amount received
            FILE *filePtr = NULL;
            filePtr = fopen("/tmp/PrimaryLikesLog", "a");
            if (filePtr == NULL) {
                #ifdef DEBUG
                printf("Something went horribly wrong trying to open the log file\n");
                #endif
                perror("Error Opening Primary Process Log File");
                exit(1);
            } else {\
                fprintf(filePtr, "Client%c %02d\n", buffer[10], likes);
                fprintf(filePtr, "Total %d\n", sum);
                fclose(filePtr);
            }
            // send the success string to alert the Like server that the likes were received
            send(cfd, SUCC, strlen(SUCC), 0);
        } else {
            // sends the fail message if the message is not valid
            send(cfd, FAIL, strlen(FAIL), 0);
        }

        #ifdef DEBUG
        printf("Server Received message: %s\n", buffer);
        #endif
        close(cfd);   
        time(&end_t);      
    }
    
    shutdown(sfd, SHUT_RDWR);
    exit(0);
}