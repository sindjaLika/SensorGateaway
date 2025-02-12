//
// Created by sindja on 8/11/23.
//
#include <stdlib.h>
#include <stdio.h>
#include "lib/dplist.h"
#include "config.h"
#include <stdint-gcc.h>
#include "datamgr.h"
#include "sbuffer.h"
#include <string.h>
#include <time.h>

//// i have a memory leak of 1 block in the routine of data manager but I could not find where the issue was.

//dplist implementation
pthread_mutex_t mutexData;
char logMessage[100];

typedef struct {
    uint16_t sensorID;
    uint16_t roomID;
    double runningAverage;
    time_t lastModified;
    sensor_value_t temperatureArray[RUN_AVG_LENGTH];
    int counter;
} my_element_t;


void * element_copy(void * element) {
    my_element_t* copy = malloc(sizeof (my_element_t));

    copy->sensorID = ((my_element_t*)element)->sensorID;
    copy->roomID = ((my_element_t*)element)->roomID;
    copy->runningAverage = ((my_element_t*)element)->runningAverage;
    copy->lastModified= ((my_element_t*)element)->lastModified;
    copy->counter= ((my_element_t*)element)->counter;
    return (void *) copy;
}

void element_free(void ** element) {
    free(*element);
    *element = NULL;
}

int element_compare(void * x, void * y) {
    return ((((my_element_t*)x)->sensorID < ((my_element_t*)y)->sensorID) ? -1 : (((my_element_t*)x)->sensorID== ((my_element_t*)y)->sensorID) ? 0 : 1);
}

//new code
dplist_t *sensorList=NULL;


void *datamgr_routine( void *arg){

    DatamgrArgs *datamgrArgs = (DatamgrArgs *)arg;
    sensorList = dpl_create(element_copy, element_free, element_compare);
    pthread_mutex_init(&mutexData,NULL);

    sbuffer_t *buffer = datamgrArgs->dataBuffer;

    FILE *fp_sensor_map = fopen("room_sensor.map", "r");

    my_element_t *new_sensor;
    // Read room-sensor mapping file
    while (1) {
        uint16_t room_id, sensor_id;

        if (fscanf(fp_sensor_map, "%hu %hu", &room_id, &sensor_id) != 2) {
            break;
        }

        new_sensor = malloc(sizeof(my_element_t));
        new_sensor->roomID= room_id;
        new_sensor->sensorID = sensor_id;
        new_sensor->counter=0;
        new_sensor->runningAverage=0.0; //setting a dumb value needed for checking the temperatures.
        sensorList = dpl_insert_at_index(sensorList, new_sensor, dpl_size(sensorList), true);
        free(new_sensor);
    }
    fclose(fp_sensor_map);

////     I test that the data manager is indeed getting all the correct data and checking if the synchronization works by
////   creating another .csv file from the data manager. This code is commented out and used only for testing purposes.

//    FILE* outputFile = fopen("testDataManager.csv", "a");

    my_element_t *sensorData;
    int i;
    while (1) {

        sensor_data_t *sensor = (sensor_data_t *) malloc(sizeof(sensor_data_t ));
        if (sbuffer_peek(buffer, sensor) != SBUFFER_SUCCESS ) {
            break;
        }

        pthread_mutex_lock(&mutexData);
        if (!isSensorIdValid(sensor->id)) {
            snprintf(logMessage, sizeof(logMessage), "Received sensor data with invalid sensor node ID %d\n", sensor->id);
            write_to_log_process(logMessage);
        }
        else{

            for(i=0;i<dpl_size(sensorList);i++){
                sensorData = dpl_get_element_at_index(sensorList,i);
                if(sensorData->sensorID==sensor->id){
                    if(sensorData->counter<RUN_AVG_LENGTH){
                        sensorData->counter++;
                    }
                    double sum=0;
                    updateTemperatureArray(sensorData->temperatureArray, sensor->value);

                    for (int j = 0; j < RUN_AVG_LENGTH; j++) {
                        if (sensorData->temperatureArray[j] != 0) {
                            sum += sensorData->temperatureArray[j];
                        }
                    }
                    sensorData->runningAverage=sensorData->counter > 0 ? sum / sensorData->counter : 0;
                    if (sensorData->runningAverage>= SET_MAX_TEMP) {
                        snprintf(logMessage, sizeof(logMessage), "Sensor node %d reports it’s too hot ( avg temp = %.2f).\n", sensorData->sensorID, sensorData->runningAverage);
                        write_to_log_process(logMessage);
                    }
                    if (sensorData->runningAverage < SET_MIN_TEMP) {
                        snprintf(logMessage, sizeof(logMessage), "Sensor node %d reports it’s too cold ( avg temp = %.2f).\n", sensorData->sensorID, sensorData->runningAverage);
                        write_to_log_process(logMessage);
                    }
                    sensorData->lastModified=sensor->ts;
                }
            }
        }
//        fprintf(outputFile, "%d,%.2f,%s", sensor->id, sensor->value,  ctime(&(sensor->ts)));
//        fflush(outputFile);
        free(sensor);
        pthread_mutex_unlock(&mutexData);

    }
//    fclose(outputFile);
    pthread_mutex_destroy(&mutexData);
    pthread_exit(NULL);
}

double updateTemperatureArray(sensor_value_t temperatures[RUN_AVG_LENGTH], sensor_value_t value) {
    if (value <= 0) {
        fprintf(stderr, "Error: Invalid temperature value (%0.2f)\n", value);
        return 0.0;
    }

    if (value != 0) {
        for (int i = 0; i < RUN_AVG_LENGTH - 1; i++) {
            temperatures[i] = temperatures[i + 1];
        }
        temperatures[RUN_AVG_LENGTH - 1] = value;
    }
    return 0;
}


void datamgr_free(){
    dpl_free(&sensorList, true);
}

uint16_t datamgr_get_room_id(sensor_id_t sensor_id){
    my_element_t *sensor;
    int i;
    for(i=0;i<dpl_size(sensorList);i++){
        sensor = dpl_get_element_at_index(sensorList,i);
        if(sensor->sensorID==sensor_id){
            printf("Sensor %d has room id = %u\n", sensor->sensorID,sensor->roomID);
            return sensor->roomID;
        }
    }
    return 0;
}

bool isSensorIdValid(sensor_id_t sensorId) {
    my_element_t *sensorData;

    for (int i = 0; i < dpl_size(sensorList); i++) {
        sensorData = dpl_get_element_at_index(sensorList, i);
        if (sensorData->sensorID == sensorId) {
            return true;
        }
    }
    return false;
}

sensor_value_t datamgr_get_avg(sensor_id_t sensor_id) {
    my_element_t *sensor;
    int i;
    for(i=0;i<dpl_size(sensorList);i++){
        sensor = dpl_get_element_at_index(sensorList,i);
        if(sensor->sensorID==sensor_id){
            printf("Running Average of Sensor ID %d =%.2f\n", sensor->sensorID, sensor->runningAverage);
            return sensor->runningAverage;
        }
    }

    return 0;
}


time_t datamgr_get_last_modified(sensor_id_t sensor_id) {
    my_element_t *sensor;
    int i;
    for(i=0;i<dpl_size(sensorList);i++){
        sensor = dpl_get_element_at_index(sensorList,i);
        if(sensor->sensorID==sensor_id){
            printf("Last modified time stamp of Sensor ID %d = %s\n",sensor->sensorID,ctime(&(sensor->lastModified)));
            return sensor->lastModified;
        }
    }
    return 0;
}

int datamgr_get_total_sensors() {
    printf("Total unique sensors=%d\n", dpl_size(sensorList));
    return dpl_size(sensorList);
}

