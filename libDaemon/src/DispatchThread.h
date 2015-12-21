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

struct dispatchThread
{
    pthread_t thread_id;

    pthread_mutex_t mutex;
    
    uint8_t shouldQuit;
    
    /* IPC Receive */
//    int r_msqid;
//    key_t r_key;

    /* Error codes*/
    int error_thread;
    int error_ipc;
    
    IPCCommunicationPort _port; // client here
};

typedef struct dispatchThread DispatchThread;

int initDispatchThread( DispatchThread *dispatch );
void releaseDispatchThread(  DispatchThread *dispatch );


void sendQuitSignal( DispatchThread *dispatch );

void *startMainLoop (void * p_data);
void dispatch_MainLoop( void* dispatcher );

uint8_t waitForThreadTerminaison( DispatchThread *dispatch );


/*
 int16_t connectToIPCWithKey( DispatchThread *dispatch , key_t key);
 int8_t sendIPCMessage(DispatchThread *dispatch, const void *message );
 uint8_t IPCIsConnected( DispatchThread *dispatch );
 */

#endif /* DispatchThread_h */
