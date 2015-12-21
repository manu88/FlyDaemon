/**
 * \file IPCMessage.h
 * \brief UAVTalk IPC definitions
 * \author Manuel Deneu
 * \version 0.1
 * \date 04/12/2015
 *
 *  PRIVATE HEADER
 */


#ifndef IPCMessage_h
#define IPCMessage_h


#include <string.h>
#include "../include/UAVTalk.h"

#define DATA_MSG_SIZE     sizeof( struct UAV_Object)

typedef struct _msgbuf
{
    long    mtype;
    pid_t   pid;
    
    struct data
    {
        uint8_t buffer[DATA_MSG_SIZE];   
    }data;
} Message_buf;


enum IPC_Message_Types
{
    IPC_DataSend              = 1,
    IPC_ProcessRegistration   = 2,
    IPC_ProcessDeregistration = 3,
    
    IPC_DataRequest           = 4,
    IPC_ProcessDidRegister    = 5,
    
    /* Reserved */
    IPC_PrivateRequest         = 100,
    IPC_PrivateRequestResponse = 101,
    IPC_PingRequest            = 102,
    IPC_PingResponse           = 103
};
/*
static inline void parseIPC( const Message_buf *message, UAVObject *obj)
{
    memcpy(obj, message->data.buffer, sizeof( UAVObject ));

}
*/

/* **** **** **** **** **** **** **** **** **** **** */

#endif /* IPCMessage_h */
