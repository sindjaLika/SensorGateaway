/**
 * \author {Sindja Lika}
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
#include <time.h>

#include "lib/dplist.h"

typedef uint16_t sensor_id_t;
typedef double sensor_value_t;
typedef time_t sensor_ts_t;         // UTC timestamp as returned by time() - notice that the size of time_t is different on 32/64 bit machine

typedef struct {
    sensor_id_t id;
    sensor_value_t value;
    sensor_ts_t ts;

} sensor_data_t;


/** write_to_log_process
 * Here we write to the logger file. Used in data manager, storage manager, connection manager.
 * @param msg the message is passed by each manager.
 * @return
 */

int write_to_log_process(char *msg);

/** end_log_process
 * Here we end the log process. Used in main.
 * @return
 */

int end_log_process();

/**isSensorValid
 * Here we iterate through the dp_list of the sensors and check if the sensor which just connected to the connection
 * manager is valid. This is done via its id. The checking is done in storage manager which does not insert data in
 * the data.csv file and the data manager which does not process the data if the sensor is invalid.
 * @param sensorId is given to check that the sensor that is connected is valid.
 * @return boolean true or false.
 */

bool isSensorIdValid(sensor_id_t sensorId);


#endif /* _CONFIG_H_ */
