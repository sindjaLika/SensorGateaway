/**
 * \author {AUTHOR}
 */
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include "config.h"
#include "lib/tcpsock.h"
#include "connmgr.h"
#include "sbuffer.h"
#include <time.h>
#include <string.h>

#define DTIMEOUT_IN_MAKEFILE 5

time_t lastSensorInsertionTime = 0;

sbuffer_t *buffer;

pthread_mutex_t mutexServer;
pthread_cond_t condServer;
int MAX_CLIENTS;
int conn_counter = 0;

int processed_connections=0;

bool connectionLogged;

typedef struct {
    tcpsock_t *client;
} ThreadArgs;

//This method handles the client, here we insert into the buffer in a thread safe manner.

void *handle_connection(void *arg) {
    ThreadArgs *threadArgs = (ThreadArgs *)arg;
    tcpsock_t *client = threadArgs->client;
    sensor_data_t data;
    int bytes, result;

    sensor_data_t *sensor = malloc(sizeof(sensor_data_t));

    do {
        // read sensor ID
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *)&data.id, &bytes);
        if (result != TCP_NO_ERROR){
            //free(sensor);
            break;
        }
        sensor->id = data.id;

        if (!connectionLogged) {
            char logMessage[100];
            snprintf(logMessage, sizeof(logMessage), " Sensor node %d has opened a new connection.\n", sensor->id);
            write_to_log_process(logMessage);
            connectionLogged = true;
        }

        // read temperature
        bytes = sizeof(data.value);
        result = tcp_receive(client, (void *)&data.value, &bytes);
        if (result != TCP_NO_ERROR) {
            //free(sensor);
            break;
        }
        sensor->value = data.value;

        // read timestamp
        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void *)&data.ts, &bytes);
        if (result != TCP_NO_ERROR){
           // free(sensor);
            break;
        }

        sensor->ts = data.ts;

        printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value,
               (long int)data.ts);

        sensor_data_t *sensor_copy = malloc(sizeof(sensor_data_t));
        memcpy(sensor_copy, sensor, sizeof(sensor_data_t));

        sbuffer_insert(buffer, sensor_copy);
        free(sensor_copy);


//        pthread_mutex_lock(&mutexServer);
//        lastSensorInsertionTime = time(NULL);
//        time_t currentTime = time(NULL);
//        double elapsedTime = difftime(currentTime, lastSensorInsertionTime);
//        printf("Elapsed time since last sensor insertion: %.2f seconds\n", elapsedTime);
//        pthread_mutex_unlock(&mutexServer);


    } while (1);

    char logMessage[100];
    snprintf(logMessage, sizeof(logMessage), " Sensor node %d has closed the connection.\n", sensor->id);
    write_to_log_process(logMessage);
    free(sensor);

    if (result == TCP_CONNECTION_CLOSED){
        //free(sensor);
        printf("Peer has closed connection\n");
    }
    else
        printf("Error occurred on connection to peer\n");
    //free(sensor);

    tcp_close(&client);

    pthread_mutex_lock(&mutexServer);
    processed_connections++;

    if(processed_connections == MAX_CLIENTS){
        pthread_cond_signal(&condServer);
    }
    printf("processed_connections : %d\n",processed_connections);
    pthread_mutex_unlock(&mutexServer);

    return NULL;
}

void *connmgr_routine(void *arg) {

    sensor_data_t *dummy = malloc(sizeof(sensor_data_t));

    ConnMgrArgs *connMgrArgs = (ConnMgrArgs *)arg;

    int PORT = connMgrArgs->PORT;
    MAX_CLIENTS = connMgrArgs->MAX_CLIENTS;
    buffer = connMgrArgs->connBuffer;

    pthread_mutex_init(&mutexServer, NULL);
    pthread_cond_init(&condServer,NULL);

    printf("Test server is started\n");

    tcpsock_t *server;
    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) {
        perror("Error opening server socket");
        exit(EXIT_FAILURE);
    }

    pthread_t threads[MAX_CLIENTS];
    ThreadArgs threadArgs[MAX_CLIENTS];
    tcpsock_t *client;

    while (1) {


        if(conn_counter >= MAX_CLIENTS){
            break;
        }

//        pthread_mutex_lock(&mutexServer);
//        time_t currentTime = time(NULL);
//
//        if (currentTime - lastSensorInsertionTime <= DTIMEOUT_IN_MAKEFILE) {
//            // Recent sensor insertion, continue running
//        } else {
//            // No recent sensor insertion, shut down the server
//            pthread_mutex_unlock(&mutexServer);
//            printf("No recent sensor insertion. Server is shutting down.\n");
//            break;
//        }
//        pthread_mutex_unlock(&mutexServer);


        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) {
            perror("Error waiting for connection");
            exit(EXIT_FAILURE);
        }

        printf("Incoming client\n");
        connectionLogged=false;

        // Create a thread to handle the connection
        pthread_mutex_lock(&mutexServer);
        if(conn_counter < MAX_CLIENTS){
            threadArgs[conn_counter].client = client;
            if (pthread_create(&threads[conn_counter], NULL, handle_connection, &threadArgs[conn_counter]) != 0) {
                perror("Error creating thread");
                exit(EXIT_FAILURE);
            }
            conn_counter++;
            printf("conn_counter: %d\n", conn_counter);

        } else{
            printf("Max number of clients reached. Ignoring new connection.\n");
            tcp_close(&client);
            pthread_mutex_unlock(&mutexServer);
            break;
        }
        pthread_mutex_unlock(&mutexServer);
    }

    pthread_mutex_lock(&mutexServer);

    while (processed_connections < MAX_CLIENTS) {
    pthread_cond_wait(&condServer, &mutexServer);
    }
    pthread_mutex_unlock(&mutexServer);


    for (int i = 0; i < MAX_CLIENTS; i++) {
        pthread_join(threads[i], NULL);
    }

    if (tcp_close(&server) != TCP_NO_ERROR) {
        perror("Error closing server socket");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_destroy(&mutexServer);
    pthread_cond_destroy(&condServer);

    printf("Test server is shutting down\n");
    dummy->id = 0;
    dummy->value = 0;
    dummy->ts = 0;
    sbuffer_insert(buffer, dummy);
    free(dummy);
    pthread_exit(NULL);
}


