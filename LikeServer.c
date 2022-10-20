#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/prctl.h>


#define DEBUG 0
#define PORT 9756
#define SUCC "Successfully received a number from Client\0"
#define FAIL "Failed to receive a valid number from Client\0"

void likeServerLog(int id, char *log) {
    // create filepath dependent on the id
    char filePath[20];
    sprintf(filePath, "/tmp/LikeServer%d", id);
    // open the file path and if fail exit with return code 1
    FILE *filePtr = NULL;
    filePtr = fopen(filePath, "a");
    if (filePtr == NULL) {
        perror("Error Opening Like Server Log File");
        exit(1);
    } else {
        fprintf(filePtr, "%s\n", log);
        fclose(filePtr);
    }
}

int likeServerSend(int likes, int id) {
    char msg[20];
    sprintf(msg, "LikeServer%d %02d\0", id, likes);
    #ifdef DEBUG
    printf("%s\n", msg);
    #endif
    int sfd, valread, cfd;
    char buffer[1024] = { 0 };
    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    socklen_t server_info_len = sizeof(address);

    // create a socket
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket Creation Failed");
        exit(1);
    }

    //create a connection to the server
    if ((cfd = connect(sfd, (struct sockaddr*)&address, server_info_len)) < 0) {
        perror("Connection to Server Failed");
        return -1;
    }

    // send the message
    send(sfd, msg, strlen(msg), 0);
    #ifdef DEBUG
    printf("Sent the message: %s", msg);
    #endif
    valread = read(sfd, buffer, 1024);
    // make sure the response is the same as the success message
    if (strcmp(buffer, SUCC) < 0) {
        return -1;
    } else {
        return 1;
    }
    #ifdef DEBUG
    printf("%s\n", buffer);
    #endif
}

void main(int argc, char **argv) {
    #ifdef DEBUG
    printf("Hello from like server");
    #endif
    int id = atoi(argv[1]);
    // sleep for a little bit waiting for the primary server to wake up
    int wake = 10 - id;
    srand(time(NULL));
    sleep(wake);
    int likes = rand() % 42;
    #ifdef DEBUG
    if (likes == 0) {
        printf("%d\n", likes);
    }
    #endif
    int sleepTime = (rand() % 4) + 1;
    time_t start_t, end_t;
    time(&start_t);
    time(&end_t);
    // run the server until it hits 5 minutes
    while (difftime(end_t, start_t) <= 300 - wake) {
        #ifdef DEBUG
        printf("Random sleep time: %d\n", sleepTime);
        printf("Hello from the LikeServer%d: The Random Number generated was %d\n", id, likes);
        #endif
        sleep(sleepTime);
        // if the like server send is positive then print to log file
        if (likeServerSend(likes, id) > 0) {
            char msg[46];
            sprintf(msg, "Successfully sent %02d likes to Primary Server\0", likes);
            likeServerLog(id, msg);
            likes = 0;
        } else {
            // print that the message failed
            char msg[43];
            sprintf(msg, "Failed to send %02d likes to Primary Server\0", likes);
            likeServerLog(id, msg);
        }
        time(&end_t);
    }

    exit(0);
}