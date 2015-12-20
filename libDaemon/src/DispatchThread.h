//
//  DispatchThread.h
//  DIspatcher
//
//  Created by Manuel Deneu on 04/12/2015.
//  Copyright © 2015 Manuel Deneu. All rights reserved.
//

#ifndef DispatchThread_h
#define DispatchThread_h

/*
 PRIVATE HEADER
 */
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
    int r_msqid;
    key_t r_key;

    /* Error codes*/
    int error_thread;
    int error_ipc;
    
    IPCCommunicationPort _port; // client here
};

typedef struct dispatchThread DispatchThread;

int initDispatchThread( DispatchThread *dispatch );

int16_t connectToIPCWithKey( DispatchThread *dispatch , key_t key);
uint8_t IPCIsConnected( DispatchThread *dispatch );

void sendQuitSignal( DispatchThread *dispatch );

void *startMainLoop (void * p_data);
void dispatch_MainLoop( void* dispatcher );

uint8_t waitForThreadTerminaison( DispatchThread *dispatch );

int8_t sendIPCMessage(DispatchThread *dispatch, const void *message );


#endif /* DispatchThread_h */
