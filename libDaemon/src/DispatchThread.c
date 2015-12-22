//
//  DispatchThread.c
//  DIspatcher
//
//  Created by Manuel Deneu on 04/12/2015.
//  Copyright © 2015 Manuel Deneu. All rights reserved.
//

#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>

#include "DispatchThread.h"
#include "IPCMessage.h"


/*
 PRIVATE FILE
 */


int initDispatchThread( DispatchThread *dispatch )
{
    if( dispatch == NULL)
        return -1;
        
    pthread_mutex_init( &dispatch->mutex, NULL);
    
    dispatch->shouldQuit = 0;



    IPC_initialize( &dispatch->_port);
    IPC_createClient( &dispatch->_port);
    
    return 1;
}

void releaseDispatchThread(  DispatchThread *dispatch )
{
    if( dispatch == NULL)
        return;
    
    pthread_mutex_destroy( &dispatch->mutex );
    
    IPC_closeClient( &dispatch->_port );

}

ALWAYS_INLINE int startThread( DispatchThread *dispatch , void *data)
{
    return pthread_create ( &dispatch->thread_id, NULL,dispatch->threadMainFunction, data );
}



/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */
/*
int16_t connectToIPCWithKey( DispatchThread *dispatch , key_t key)
{
    if( dispatch->r_key != -1 )
        return 0; // already connected
    
    if (( dispatch->r_msqid = msgget( key, 0666)) < 0)
    {
        perror("msgget");
        dispatch->error_ipc = dispatch->r_msqid;
        return -1; // unable to connect
    }
    
    dispatch->r_key = key;
    
    return 1;
    
}

uint8_t IPCIsConnected( DispatchThread *dispatch )
{
    return dispatch->r_key > 0;
}
*/


/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */



/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

uint8_t waitForThreadTerminaison( DispatchThread *dispatch )
{
    const int ret = pthread_join ( dispatch->thread_id, NULL);
    
    return ret == 0? 1 : 0;
}



