/**
 * \author {Sindja Lika}
 */

#ifndef _SBUFFER_H_
#define _SBUFFER_H_

#include "config.h"
#include <pthread.h>

#define SBUFFER_FAILURE -1
#define SBUFFER_SUCCESS 0
#define SBUFFER_NO_DATA 1



typedef struct sbuffer sbuffer_t;



/**
 * Allocates and initializes a new shared connBuffer
 * \param buffer a double pointer to the connBuffer that needs to be initialized
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occurred
 */
int sbuffer_init(sbuffer_t **buffer);

/**
 * All allocated resources are freed and cleaned up
 * \param buffer a double pointer to the connBuffer that needs to be freed
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occurred
 */
int sbuffer_free(sbuffer_t **buffer);

/**
 * Removes the first sensor data in 'connBuffer' (at the 'head') and returns this sensor data as '*data'
 * If 'connBuffer' is empty, the function doesn't block until new sensor data becomes available but returns SBUFFER_NO_DATA
 * Here we check if the data manager has peeked and also detected the end of the buffer.
 * \param buffer a pointer to the connBuffer that is used
 * \param data a pointer to pre-allocated sensor_data_t space, the data will be copied into this structure. No new memory is allocated for 'data' in this function.
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occurred
 */
int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data);

/**
 * Inserts the sensor data in 'data' at the end of 'connBuffer' (at the 'tail')
 * \param buffer a pointer to the connBuffer that is used
 * \param data a pointer to sensor_data_t data, that will be copied into the connBuffer
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occured
*/
int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data);

/**peek
 * THis method is used to only peek and not remove the data.
 * @param buffer is the shared buffer
 * @param data  is the data obtained
 * @return
 */
int sbuffer_peek(sbuffer_t *buffer, sensor_data_t *data);
#endif  //_SBUFFER_H_
