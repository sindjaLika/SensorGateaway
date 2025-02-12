//
// Created by sindja on 20/12/23.
//
/**
 * \author {Sindja Lika}
 */

#ifndef STUDENTSOURCE2023_CONNMGR_H
#define STUDENTSOURCE2023_CONNMGR_H
#include "sbuffer.h"

//I add a struct in the connmgr.h in order to pass the arguments as they should from the main.
/**struct
 * We define a struct in order to pass the arguments from main to connection manager.
 * Port is the port of the server
 * MAX_CLIENTS is the max number of the clients
 * connBuffer is the shared buffer.
 */

typedef struct {
    int PORT;
    int MAX_CLIENTS;
    sbuffer_t *connBuffer;
} ConnMgrArgs;

/**handle_connection
 * This routine is created to be used by the local created threads of the connection manager.
 * These threads process the client data and inform the server when they close the connection.
 * THis is done via processed_connections , mutexes are used to do a thread safe increments of
 * this parameter, that is then later checked if it reaches MAX_CLIENTS and let the server know when to shut down.
 * write_to_log is called here.
 * @param arg
 * @return
 */

void *handle_connection(void *arg);

/**connmgr_routine
 * This routine is passed to the connection manager thread in the main. IN this routine , the connection manager server
 * waits for new TCP connections and checks how many clients it should allow to give data.
 * This is done via a counter which is incremented in a thread safe manner via mutexes. Also here the server shut down
 * is done.
 * @param arg
 * @return
 */
void *connmgr_routine(void *arg);

#endif //STUDENTSOURCE2023_CONNMGR_H
