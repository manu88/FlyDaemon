//
//  IPCMessage.h
//  Dispatcher
//
//  Created by Manuel Deneu on 04/12/2015.
//  Copyright © 2015 Manuel Deneu. All rights reserved.
//

#ifndef IPCMessage_h
#define IPCMessage_h

/*
    PRIVATE HEADER
 */
#include <string.h>
#include "../UAVTalk.h"

// this is PRIVATE !!
#define IPC_KEY  (key_t) 1235


#define MSGSZ     sizeof( struct UAV_Object)

typedef struct _msgbuf
{
    long    mtype;
    pid_t   pid;
    
    struct data
    {
        uint8_t buffer[MSGSZ];   
    }data;
} Message_buf;


enum IPC_Message_Types
{
    IPC_DataSend              = 1,
    IPC_ProcessRegistration   = 2,
    IPC_ProcessDeregistration = 3,
    
    IPC_DataRequest           = 4,
    IPC_ProcessDidRegister    = 5
};

static inline void parseIPC( const Message_buf *message, UAVObject *obj)
{
    memcpy(obj, message->data.buffer, sizeof( UAVObject ));

}


/* **** **** **** **** **** **** **** **** **** **** */

#endif /* IPCMessage_h */
