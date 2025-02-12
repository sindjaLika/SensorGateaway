/**
 * \author {Sindja Lika}
 */

#ifndef _SENSOR_DB_H_
#define _SENSOR_DB_H_

#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "sbuffer.h"
#include <stdbool.h>

//we need to pass the buffer as it is shared.
/**Struct:
 * We define a struct in order to pass the same buffer from main and storage manager. We use sbuffer struct.
 */
typedef struct {
    sbuffer_t *storageBuffer;
} StoragemgrArgs;

/** Storage manager routine
 * Here we remove the node after we know that the data manager has peeked.
 * We call this routine for the storage manager threat created in main. In this routine a new data.csv file is generated
 * after each run. This thread is synchronized with the connection manager and data manager.
 * @param args gets the shared buffer from the main.
 * @return
 */

void* storagemgr_routine(void* args);

/**open_db
 * Here we open a file and we select whether we want to append to it or write over it. This is used in the previous
 * routine and the main in order to create the logger file
 * @param filename the file name of data.csv
 * @param append here we pass a boolean whether we want to add new data or over write what is in the file.
 * @return
 */

FILE* open_db(char * filename, bool append);

/**close_db
 * Here we close the file. Used by the previous routine and main.
 * @param f is the file
 * @return
 */

int close_db(FILE * f);

#endif /* _SENSOR_DB_H_ */