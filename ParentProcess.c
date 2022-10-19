#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/prctl.h>

#define PORT 9756
#define SUCC "Hello from Server\0"

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

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket Creation Failed");
        exit(1);
    }

    if ((cfd = connect(sfd, (struct sockaddr*)&address, server_info_len)) < 0) {
        perror("Connection to Server Failed");
        return -1;
    }

    send(sfd, msg, strlen(msg), 0);
    #ifdef DEBUG
    printf("Sent the message: %s", msg);
    #endif
    valread = read(sfd, buffer, 1024);
    if (strcmp(buffer, SUCC) < 0) {
        return -1;
    } else {
        return 1;
    }
    #ifdef DEBUG
    printf("%s\n", buffer);
    #endif
}

void likeServer(int id){
    // sleep for a little bit waiting for the primary server to wake up
    int wake = 10 - id;
    sleep(wake);
    srand(time(NULL) + id);
    int likes = rand() % 42;
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
        if (likeServerSend(likes, id) > 0) {
            likes = 0;
            char msg[46];
            sprintf(msg, "Successfully sent %02d likes to Primary Server\0", likes);
            likeServerLog(id, msg);
        } else {
            char msg[43];
            sprintf(msg, "Failed to send %02d likes to Primary Server\0", likes);
            likeServerLog(id, msg);
        }
        time(&end_t);
    }

    exit(0);
}

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
    int value = atoi(msg+12);
    printf("%d\n", value);
    return value;
}

void primaryLikeServer() {
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
            sum += likes;
            printf("%d\n", sum);
            // print out the total and the amount received
            FILE *filePtr = NULL;
            filePtr = fopen("/tmp/PrimaryLikesLog", "a");
            if (filePtr == NULL) {
                printf("Something went horribly wrong trying to open the log file\n");
                perror("Error Opening Primary Process Log File");
                exit(1);
            } else {\
                fprintf(filePtr, "Client%c %02d\n", buffer[10], likes);
                fprintf(filePtr, "Total %d\n", sum);
                fclose(filePtr);
            }
        }

        #ifdef DEBUG
        printf("Server Received message: %s\n", buffer);
        #endif
        // send the success string to alert the Like server that the likes were received
        send(cfd, SUCC, strlen(SUCC), 0);
        close(cfd);   
        time(&end_t);      
    }
    
    shutdown(sfd, SHUT_RDWR);
    exit(0);
}

int main(int argc, char **argv) {
    // start all the processes and get them proper id to have easier management later
    int id = -1;
    pid_t pid[11];
    for (int ii = 0; ii < 11; ii++) {
        pid[ii] = fork();
        if (pid[ii] == 0) {
            id = ii;
            #ifdef DEBUG
            if (ii < 10) {
                printf("LikeServer%d is running\n", id);
            } else {
                printf("PrimaryLikeServer is Alive\n");
            }
            #endif
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
                }
                fclose(filePtr);
            }
        }
        sleep(1);
    }

    int argvLength = strlen(argv[0]);
    if (id < 10 && id > -1) {
        char name[12];
        // set the name of the process to LikeServer#
        sprintf(name, "LikeServer%d", id);
        strncpy(argv[0], name, strlen(argv[0]));
        prctl(PR_SET_NAME, name, 0, 0, 0);
        // call the actual functionality of the like servers
        likeServer(id);
        // exit with 1 if the program goes horribly wrong
        exit(1);
    } else if (id > 9) {
        // Set the name of the process to be 16 bytes of PrimaryLikesServer
        strncpy(argv[0], "PrimaryLikesServer\0", strlen(argv[0]));
        prctl(PR_SET_NAME, "PrimaryLikesServer\0", 0, 0, 0);
        // run the actual functionality of Primary Likes Server
        primaryLikeServer();
        // exit with 1 if anything gets skipped
        exit(1);
    } else {
        int stat;
        for (int ii = 0; ii < 11; ii++) {
            pid_t tempPid = waitpid(pid[ii], &stat, 0);
            if (WIFEXITED(stat)) {
                int exit = WEXITSTATUS(stat);
                #ifdef DEBUG
                printf("LikeServer%d terminated with status: %d\n", ii, exit);
                #endif
                FILE *filePtr = NULL;
                filePtr = fopen("/tmp/ParentProcessStatus", "a");
                if (filePtr == NULL) {
                    perror("Error Opening Parent Process Log File");
                } else {
                    if (exit > 0) {
                        if (ii > 9) {
                            fprintf(filePtr, "The Primary Likes Server Failed with exit status: %d\n", exit);
                        } else {
                            fprintf(filePtr, "LikeServer%d Failed with exit status: %d\n", ii, exit);
                        }
                    } else {
                        if (ii > 9) {
                            fprintf(filePtr, "The Primary Likes Server Succeeded and exited with exit status: %d\n", exit);
                        } else {
                            fprintf(filePtr, "LikeServer%d Succeeded with exit status: %d\n", ii, exit);
                        }
                    }
                    fclose(filePtr);
                }
            }
        }
    }
    return 0;
}