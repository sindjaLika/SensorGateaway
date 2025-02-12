//
// Created by sindja on 19/12/23.
//

#include "connmgr.h"
#include "datamgr.h"
#include "sbuffer.h"
#include "sensor_db.h"
#include "config.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define THREAD_NR 3

sbuffer_t *mainBuffer;

#define LOG_FILE "gateway.log"

FILE *logFile;
int pipeLog[2];
int sequenceNr=0;
char fullMessage[256];
int idFork;


int main(int argc, char *argv[]) {

   // create_log_process();

    if(pipe(pipeLog)==-1){
        printf("ERROR CREATING THE PIPE");
    }

    //forking here to make the child work

    idFork=fork();

    if(idFork==0) {
        logFile= open_db(LOG_FILE,false);
        if(logFile==NULL){
            printf("ERROR OPENING THE LOG FILE");
        }
        close(pipeLog[1]);

        while ((read(pipeLog[0], fullMessage, sizeof(fullMessage)))>0) {

            char *logEvent=strtok(fullMessage,"\n");
            while(logEvent !=NULL){
                time_t currentTime;
                time(&currentTime);
                struct tm *timeInfo = localtime(&currentTime);
                //Formatting the current time as it is in the example.gateway.log file
                char timeStamp[64];
                strftime(timeStamp, sizeof(timeStamp), "%a %b %d %H:%M:%S %Y", timeInfo);
                fprintf(logFile, "%d - %s - %s\n",  sequenceNr++,timeStamp, logEvent);
                logEvent=strtok(NULL,"\n");
            }
            memset(fullMessage,0,sizeof(fullMessage));
        }
        close_db(logFile);
        close(pipeLog[0]);
        logFile = NULL;
        exit(1);
    }
    //the parent process
    else{

    if (argc < 3) {
        printf("Please provide the port number and the maximum number of connections\n");
        exit(EXIT_FAILURE);
    }

    ConnMgrArgs *connMgrArgs = (ConnMgrArgs *)malloc(sizeof(ConnMgrArgs));
    DatamgrArgs *datamgrArgs = (DatamgrArgs *)malloc(sizeof(DatamgrArgs));
    StoragemgrArgs *storagemgrArgs = (StoragemgrArgs *)malloc(sizeof(StoragemgrArgs));

    if (connMgrArgs == NULL) {
        perror("Error allocating memory for ConnMgrArgs");
        exit(EXIT_FAILURE);
    }

    connMgrArgs->PORT = atoi(argv[1]);
    connMgrArgs->MAX_CLIENTS = atoi(argv[2]);

    pthread_t th[THREAD_NR];

    if (sbuffer_init(&mainBuffer) != SBUFFER_SUCCESS) {
        perror("Failed to initialize shared mainBuffer");
        free(connMgrArgs);
        exit(EXIT_FAILURE);
    }
    connMgrArgs->connBuffer = mainBuffer;
    datamgrArgs->dataBuffer = mainBuffer;
    storagemgrArgs->storageBuffer= mainBuffer;

    if (pthread_create(&th[0], NULL, &connmgr_routine, (void *)connMgrArgs) != 0) {
        perror("Failed to create thread");
    }


    if(pthread_create(&th[1], NULL, &datamgr_routine, datamgrArgs) != 0){
        perror("Failed to create thread");
    }

    if(pthread_create(&th[2], NULL, &storagemgr_routine, storagemgrArgs) != 0){
        perror("Failed to create thread");
    }

    int i;

    for (i = 0; i < THREAD_NR; i++) {
        if (pthread_join(th[i], NULL) != 0) {
            perror("Failed to join thread");
        }else {
            printf("Thread %d joined successfully\n", i);
        }
    }

    free(connMgrArgs);
    free(datamgrArgs);
    free(storagemgrArgs);

    //testing purposes
    datamgr_get_room_id(37);
    datamgr_get_avg(15);
    datamgr_get_avg(21);
    datamgr_get_avg(37);

    datamgr_get_total_sensors();

    datamgr_get_last_modified(37);

    end_log_process();

    datamgr_free();

    if (sbuffer_free(&mainBuffer) != SBUFFER_SUCCESS) {
        perror("Failed to free shared mainBuffer");
        exit(EXIT_FAILURE);
    }
    printf("END PROJECT\n");
    //exiting the main thread
    pthread_exit(NULL);
    //return EXIT_SUCCESS;
    }
}



int write_to_log_process(char *msg) {
    write(pipeLog[1], msg, strlen(msg));
    return 0;
}


int end_log_process() {
    close(pipeLog[1]);
    waitpid(idFork, NULL, 0);
    if (logFile != NULL) {
        fclose(logFile);
        logFile = NULL;
    }
    return 0;
}