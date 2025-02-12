/**
 * \author {Sindja Lika}
 */

#ifndef DATAMGR_H_
#define DATAMGR_H_

#include <stdlib.h>
#include <stdio.h>
#include "config.h"

#ifndef RUN_AVG_LENGTH
#define RUN_AVG_LENGTH 5

#endif

#ifndef SET_MAX_TEMP
//#warning "SET_MAX_TEMP not set - using default value of 100"#define SET_MAX_TEMP 25
#define SET_MAX_TEMP 25
#endif

#ifndef SET_MIN_TEMP
#define SET_MIN_TEMP 0
//#warning "SET_MIN_TEMP not set - using default value of 0"

#endif

/*
 * Use ERROR_HANDLER() for handling memory allocation problems, invalid sensor IDs, non-existing files, etc.
 */
#define ERROR_HANDLER(condition, ...)    do {                       \
                      if (condition) {                              \
                        printf("\nError: in %s - function %s at line %d: %s\n", __FILE__, __func__, __LINE__, __VA_ARGS__); \
                        exit(EXIT_FAILURE);                         \
                      }                                             \
                    } while(0)

/**
 *  This method holds the core functionality of your datamgr. It takes in 2 file pointers to the sensor files and parses them. 
 *  When the method finishes all data should be in the internal pointer list and all log messages should be printed to stderr.
 *  \param fp_sensor_map file pointer to the map file
 *  \param fp_sensor_data file pointer to the binary data file
 */
#include "sbuffer.h"
//we need to pass the same buffer

/**struct
 * here we create a struct to pass the buffer from the main so the buffer is shared.
 */
typedef struct {
    sbuffer_t *dataBuffer;
} DatamgrArgs;

/** datamgr_routine
 * The data manager is set to peek and not to remove the node. It will peek and then let the storage manager know that
 * it can remove now. This way they are synchronized. The peek method is declared in sbuffer.h
 * Here in this routine we first read the room-map file and create a dp_list with all the unique sensor ids and
 * their respective room number. In this specific case we get a dp_list with 8 nodes. Then we process the data we get
 * from the shared buffer. First we check if the sensor id is valid by the isSensorIdValid method , if it is
 * we continue processing the data by checking which respective node it belongs to so iterating through the previously
 * created dp_list and update a running average of the 5 last temperature values. The array of the values and a counter is added
 * to the parameters that the dp_list node has . The calculation is done via the counter. Basically this counter keeps track
 * of the values inserted till the defined RUN_AVG_LENGTH. And then we update the sum by adding the elements and then
 * we divide the two to get the running average. Then the checking of the temperatures is added, if it is too cold or too
 * hot. The write to log process is called here. The last modified time stamp is also modified each time in the loop here for evrey new
 * incoming sensors.
 * @param arg
 * @return
 */
void *datamgr_routine(void *arg);

/**
 * This method should be called to clean up the datamgr, and to free all used memory. 
 * After this, any call to datamgr_get_room_id, datamgr_get_avg, datamgr_get_last_modified or datamgr_get_total_sensors will not return a valid result
 */
void datamgr_free();

/**
 * Gets the room ID for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the corresponding room id
 */
uint16_t datamgr_get_room_id(sensor_id_t sensor_id);

/**
 * Gets the running AVG of a certain senor ID (if less then RUN_AVG_LENGTH measurements are recorded the avg is 0)
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the running AVG of the given sensor
 */
sensor_value_t datamgr_get_avg(sensor_id_t sensor_id);

/**
 * Returns the time of the last reading for a certain sensor ID
 * Use ERROR_HANDLER() if sensor_id is invalid
 * \param sensor_id the sensor id to look for
 * \return the last modified timestamp for the given sensor
 */
time_t datamgr_get_last_modified(sensor_id_t sensor_id);

/**
 *  Return the total amount of unique sensor ID's recorded by the datamgr
 *  \return the total amount of sensors
 */
int datamgr_get_total_sensors();

/**updateTemperatureArray
 * here we update the temperature Arrays for each sensor. We want all the last values depending on
 * running average, which in this case is 5 if not defined. So the last 5 temperature values are calculated
 * this is done via shifting all the elements when a new value is added.
 * @param temperatures is the array we change.
 * @param value is the value we pass for each sensor we get from the shared buffer.
 * @return
 */

double updateTemperatureArray(sensor_value_t temperatures[RUN_AVG_LENGTH], sensor_value_t value);

/**
 * All these methods are created from the lab.
 * @param element
 * @return
 */
void* element_copy(void * element);
void element_free(void ** element);
int element_compare(void * x, void * y);


#endif  //DATAMGR_H_
