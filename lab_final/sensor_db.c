//
// Created by sindja on 17/11/23.
//


#include "sensor_db.h"
#include <stdio.h>
#include "sbuffer.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "config.h"

pthread_mutex_t mutexStorage;


void* storagemgr_routine(void* arg){
    StoragemgrArgs *storagemgrArgs = (StoragemgrArgs *)arg;
    sensor_data_t *sensor = (sensor_data_t *) malloc(sizeof(sensor_data_t ));
    sbuffer_t *buffer = storagemgrArgs->storageBuffer;
    pthread_mutex_init(&mutexStorage,NULL);

    FILE *outputFile = open_db("data.csv", false);
    write_to_log_process("A new data.csv file has been created.\n");
    if (outputFile == NULL) {
        perror("Failed to open data.csv");
        pthread_exit(NULL);
    }

    while (1) {

        if (sbuffer_remove(buffer, sensor) != SBUFFER_SUCCESS) {
            break;
        }

        pthread_mutex_lock(&mutexStorage);
        // Write sensor data to CSV file
        if (isSensorIdValid(sensor->id)) {
            fprintf(outputFile, "%d,%.2f,%s", sensor->id, sensor->value,  ctime(&(sensor->ts)));
            fflush(outputFile);
            char logMessage[100];
            snprintf(logMessage, sizeof(logMessage), " Data insertion from sensor %d succeeded.\n", sensor->id);
            write_to_log_process(logMessage);
        }

        pthread_mutex_unlock(&mutexStorage);

    }
    close_db(outputFile);
    write_to_log_process("The data.csv file has been closed.\n");
    free(sensor);
    pthread_mutex_destroy(&mutexStorage);
    pthread_exit(NULL);
    //return NULL;
}

FILE* open_db(char * filename, bool append){
    FILE* file;
    if (append){
        file= fopen(filename,"a");
    }
    else{
        file=fopen(filename,"w");
    }
    if(file==NULL) {
        write_to_log_process("Error opening the csv file.\n");
        return NULL;
    }
    write_to_log_process("Data file opened.\n");
    return file;
}


int close_db(FILE * f){
    fclose(f);
    write_to_log_process("Data file closed.\n");
    end_log_process();
    return 0;
}
