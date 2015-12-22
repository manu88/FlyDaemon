/**
 * \file DispatchThread.h
 * \brief Internal dispatcher thread
 * \author Manuel Deneu
 * \version 0.1
 * \date 04/12/2015
 *
 *  PRIVATE HEADER
 */

#ifndef DispatchThread_h
#define DispatchThread_h

#include <stdint.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>

#include "../../PrivateAPI/IPCComm.h"
#include "../include/Commons.h"

typedef void *(*threadMain)( void *data);

typedef struct
{
    pthread_t thread_id;

    threadMain threadMainFunction;
    
    pthread_mutex_t mutex;
    
    uint8_t shouldQuit;
    
    IPCCommunicationPort _port; // client here
    
    
    /* IPC Receive */
    //    int r_msqid;
    //    key_t r_key;
} DispatchThread;


int initDispatchThread( DispatchThread *dispatch );
void releaseDispatchThread(  DispatchThread *dispatch );


ALWAYS_INLINE int startThread( DispatchThread *dispatch , void*data);


ALWAYS_INLINE void sendQuitSignal( DispatchThread *dispatch );


BOOLEAN_RETURN uint8_t waitForThreadTerminaison( DispatchThread *dispatch );


/*
 int16_t connectToIPCWithKey( DispatchThread *dispatch , key_t key);
 int8_t sendIPCMessage(DispatchThread *dispatch, const void *message );
 uint8_t IPCIsConnected( DispatchThread *dispatch );
 */

#endif /* DispatchThread_h */
