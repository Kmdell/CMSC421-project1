#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT 9756
#define SUCC "Successfully received a number from Client\0"
#define FAIL "Failed to receive a valid number from Client\0"

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
            if (ii < 10) {
                char id[1];
                sprintf(id, "%d", ii);
                if (execlp("./LikeServer", "./LikeServer", id, NULL) < 0) {
                    perror("Error executing a LikeServer binary in the process");
                    exit(1);
                }
            } else {
                if (execlp("./PrimaryLikesServer", "PrimaryLikesServer", NULL) < 0) {
                    perror("Error executing the Primary Likes Server binary in the process");
                    exit(1);
                }
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
                }
                fclose(filePtr);
            }
        }
        sleep(1);
    }

    
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
    return 0;
}