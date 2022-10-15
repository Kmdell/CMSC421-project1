#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#define PORT 9765
#define HELLO "Hello from Server\n"

int likeServerSend(int likes, int id) {
    char 
    printf("");
    return 0;
}

void likeServer(int id){
    srand(time(NULL) + id);
    int likes = rand() % 42;
    int sleepTime = (rand() % 4) + 1;
    time_t start_t, end_t;
    time(&start_t);
    time(&end_t);
    while(difftime(end_t, start_t) > 300) {
        sleep(sleepTime);
        if (likeServerSend(likes, id) > 0) {
            likes = rand() % 42;
        }
        time(&end_t);
    }
    printf("Hello from the LikeServer%d: The Random Number generated was %d\n", id, random);
    exit(0);
}

void primaryLikeServer() {
    printf("Hello from the primary like server\n");
    // declare the variables for socket programming
    int sfd, cfd, valread;
    char buffer[1024] = { 0 };
    struct sockaddr_in address = { 0 };
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    socklen_t server_info_len = sizeof(address);

    // Initialize the socket using IPv4 and TCP
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("Socket Failed to initialize");
        exit(1);
    }

    // bind on the 9765 port number
    if (bind(sfd, (struct sockaddr*) &address, server_info_len) < 0) {
        perror("Error binding to to port");
        exit(1);
    }

    // start listening on the socket
    if (listen(sfd, 0) < 0) {
        perror("Error while starting to listen");
        exit(1);
    }

    // accept the connection and keep the socket info for the client to send a response
    if ((cfd = accept(sfd, (struct sockaddr*) &address, (socklen_t*)&server_info_len)) < 0) {
        perror("Error while accepting connection");
        exit(1);
    }

    // read the buffer then print and reply with the hello message
    valread = read(cfd, buffer, 1024);
    printf("%s\n", buffer);
    send(cfd, HELLO, strlen(HELLO), 0);
    close(cfd);

    exit(0);
}

int main() {
    // start all the processes and get them proper id to have easier management later
    int id = -1;
    pid_t pid[11];
    for (int ii = 0; ii < 11; ii++) {
        pid[ii] = fork();
        if (pid[ii] == 0) {
            id = ii;
            if (ii < 10) {
                printf("LikeServer%d is running\n", id);
            } else {
                printf("PrimaryLikeServer is Alive\n");
            }
            ii = 100;
        } else if (pid[ii] > 0) {
            // print out that the like servers have been started
            FILE *filePtr = NULL;
            filePtr = fopen("/tmp/ParentProcessStatus", "a");
            if (filePtr == NULL) {
                perror("Error Opening Parent Process Log File");
            } else {
                if (ii < 10) {
                    fprintf(filePtr, "The Child Process for LikeServer%d was started with PID %d\n", ii, pid[ii]);
                } else {
                    fprintf(filePtr, "The Child Process for PrimaryLikeServer was started with PID %d\n", pid[ii]);
                }
                fclose(filePtr);
            }
        } else {
            // print logs if the processes do not start for the servers
            perror("There was an error encountered\n");
            FILE *filePtr = NULL;
            filePtr = fopen("/tmp/ParentProcessStatus", "a");
            if (filePtr == NULL) {
                perror("Error Opening Parent Process Log File");
            } else {
                if (ii < 10) {
                    fprintf(filePtr, "LikeServer%d did not start properly\n", ii);
                } else {
                    fprintf(filePtr, "The PrimaryLikeServer did not start properly");
                }FILE *filePtr = NULL;
            filePtr = fopen("/tmp/ParentProcessStatus", "a");
            if (filePtr == NULL) {
                perror("Error Opening Parent Process Log File");
            } else {
                if (ii < 10) {
                    fprintf(filePtr, "The Child Process for LikeServer%d was started with PID %d\n", ii, pid);
                } else {
                    fprintf(filePtr, "The Child Process for PrimaryLikeServer was started with PID %d\n", pid);
                }
                fclose(filePtr);
            }
                fclose(filePtr);
            }
        }
    }

    if (id < 10 && id > -1) {
        likeServer(id);
        exit(1);
    } else if (id > 9) {
        primaryLikeServer();
        exit(1);
    } else {
        int stat;
        for (int ii = 0; ii < 11; ii++) {
            pid_t tempPid = waitpid(pid[ii], &stat, 0);
            if (WIFEXITED(stat)) {
                printf("LikeServer%d terminated with status: %d\n", ii, WEXITSTATUS(stat));
            }
        }
    }
    return 0;
}