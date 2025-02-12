/**
 * \author {AUTHOR}
 */

#include <stdlib.h>
#include "sbuffer.h"
#include <pthread.h>
#include <string.h>
#include "config.h"


/**
 * basic node for the connBuffer, these nodes are linked together to create the connBuffer
 */
//I include a variable peeked to let the storage manager know that the data manager has obtained the data before removing it.
int finished=0;
typedef struct sbuffer_node {
    struct sbuffer_node *next;  /**< a pointer to the next node*/
    sensor_data_t data;         /**< a structure containing the data */
    int peeked;
} sbuffer_node_t;


/**
 * a structure to keep track of the connBuffer
 */
struct sbuffer {
    sbuffer_node_t *head;       /**< a pointer to the first node in the connBuffer */
    sbuffer_node_t *tail;       /**< a pointer to the last node in the connBuffer */
    pthread_mutex_t mutexBuffer;
    pthread_cond_t bufferNotEmpty;
};


int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    pthread_mutex_init(&((*buffer)->mutexBuffer), NULL);
    pthread_cond_init(&((*buffer)->bufferNotEmpty), NULL);

    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t **buffer) {
    sbuffer_node_t *dummy;
    if ((buffer == NULL) || (*buffer == NULL)) {
        return SBUFFER_FAILURE;
    }

    while ((*buffer)->head) {
        dummy = (*buffer)->head;
        (*buffer)->head = (*buffer)->head->next;
        free(dummy);
    }

    pthread_mutex_destroy(&(*buffer)->mutexBuffer);
    pthread_cond_destroy(&((*buffer)->bufferNotEmpty));

    free(*buffer);
    *buffer = NULL;
    return SBUFFER_SUCCESS;
}


//here i peek and remove
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data) {

    pthread_mutex_lock(&(buffer->mutexBuffer));
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE;

    //here I check if the data manager has peeked in order to remove.
    while (buffer->head == NULL || buffer->head->peeked ==0 ) {
        pthread_cond_wait(&(buffer->bufferNotEmpty), &(buffer->mutexBuffer));
    }

    *data = buffer->head->data;

        if ((*data).id == 0 || finished) {
            pthread_mutex_unlock(&(buffer->mutexBuffer));
            pthread_cond_signal(&(buffer->bufferNotEmpty));
            return SBUFFER_NO_DATA;
        }

        dummy = buffer->head;

        if (buffer->head == buffer->tail) // buffer has only one node
        {
            buffer->head = buffer->tail = NULL;
        } else  // buffer has many nodes empty
        {
            buffer->head = buffer->head->next;
        }
        free(dummy);

    pthread_mutex_unlock(&(buffer->mutexBuffer));

    return SBUFFER_SUCCESS;
}

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {


    if (buffer == NULL) return SBUFFER_FAILURE;

    pthread_mutex_lock(&(buffer->mutexBuffer));

    sbuffer_node_t *dummy;
    dummy = malloc(sizeof(sbuffer_node_t));

    if (dummy == NULL){
        pthread_mutex_unlock(&(buffer->mutexBuffer));
        return SBUFFER_FAILURE;}


    dummy->data = *data;

    dummy->next = NULL;

    if (buffer->tail == NULL)
    {
        buffer->head = buffer->tail = dummy;
    } else
    {
        buffer->tail->next = dummy;
        buffer->tail = buffer->tail->next;
    }
    //we initiate the peeked here for each data
    dummy->peeked=0;

    pthread_cond_broadcast(&(buffer->bufferNotEmpty));
    pthread_mutex_unlock(&(buffer->mutexBuffer));

    return SBUFFER_SUCCESS;
}

int sbuffer_peek(sbuffer_t *buffer, sensor_data_t *data) {

    pthread_mutex_lock(&(buffer->mutexBuffer));

    if (buffer == NULL) return SBUFFER_FAILURE;

    while (buffer->head == NULL || buffer->head->peeked==1 ) {
        pthread_cond_wait(&(buffer->bufferNotEmpty), &(buffer->mutexBuffer));
    }

    *data = buffer->head->data;
    //we set it here to make the storage manager realise that data manager has peeked.
    buffer->head->peeked = 1;


    if( data->id==0){

        finished=1;
        pthread_mutex_unlock(&(buffer->mutexBuffer));
        pthread_cond_signal(&(buffer->bufferNotEmpty));
        return SBUFFER_NO_DATA;
    }

   pthread_cond_signal(&(buffer->bufferNotEmpty));
    pthread_mutex_unlock(&(buffer->mutexBuffer));
    return SBUFFER_SUCCESS;
}

