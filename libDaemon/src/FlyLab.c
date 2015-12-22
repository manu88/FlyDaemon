//
//  FlyLab.c
//  Client
//
//  Created by Manuel Deneu on 18/12/2015.
//  Copyright © 2015 Manuel Deneu. All rights reserved.
//




/*
 PRIVATE HEADER
 */
#include <unistd.h>
#include <stdlib.h> // pour free() : pas bien!
#include <stddef.h>
#include <stdio.h>
#include "../include/FlyLabAPI.h"
#include "Dispatch.h"
#include "IPCMessage.h"


void eventReceived( int reason, const void* msg, void* data);
int8_t dispatchMainLoop(void *data);

static const char _version[] = "0.0.1";

static RuntimeInformations _runtimeInfos;
static GrandDispatcher *instance = NULL;
static FlyLabParameters params;

static uint8_t _connected = 0;

BOOLEAN_RETURN uint8_t initializeConnection( const FlyLabParameters *parameters )
{
    
    memset( &_runtimeInfos, 0, sizeof(RuntimeInformations ));
    
    params = *parameters;
    
    instance = GD_init();
    
    _connected = 0;
    
    GD_setCallBack1( instance, eventReceived, parameters->userData );

    GD_setDispatchMethod( instance , dispatchMainLoop , instance );
    
    
    return instance != NULL;
}

void cleanup()
{
    if( instance->state > 0)
        GD_stop( instance);
    
    if (instance != NULL)
        GD_release( instance );
}

ALWAYS_INLINE int8_t getReturnValue()
{
    return instance->returnValue;
}

ALWAYS_INLINE const char* API_getVersion()
{
    return _version;
}

ALWAYS_INLINE uint8_t informationsAvailable()
{
    return _runtimeInfos.plateform != Plateform_Unknown;
}

const RuntimeInformations * getRuntimeInformations( )
{
    if (informationsAvailable() )
        return &_runtimeInfos;
    
    return NULL;
}

BOOLEAN_RETURN uint8_t requestRuntimeInformationsUpdate( void )
{
    Message_buf sbuf;
    
    sbuf.mtype = IPC_PrivateRequest;
    return IPC_send( &instance->_thread._port  , &sbuf, sizeof(Message_buf )) >0?1 : 0;
}

ALWAYS_INLINE uint8_t disconnect()
{
    return GD_stop( instance );
}

uint8_t runFromThisThread()
{
    return GD_runFromLoop( instance ) == 0? 1 : 0;
}
uint8_t runFromNewThread()
{
    if( GD_runFromThread( instance )  == 0 )
        return 0;
    
    if( GD_waitForCreation( instance ) == 0 )
        return 0;
        
    return instance->state >= 1;
}

ALWAYS_INLINE uint8_t isConnected()
{
    return _connected;// instance->state >= 1;
}

ALWAYS_INLINE int8_t sendObject( const UAVObject * obj)
{
    return GD_sendMessage( instance ,&obj , sizeof( UAVObject ))  ;
}

int8_t sendObjectRequest( uint32_t objectID)
{
    UAVObject obj;
    createREQ(&obj, objectID);
    
    return GD_sendMessage( instance ,&obj , sizeof( UAVObject ))  ; //

}

ERROR_RETURN int8_t respondAcknowledge( uint16_t instanceID )
{
    UAVObject obj;
    createACK(&obj, instanceID );
    
    return GD_sendMessage( instance ,&obj , sizeof( UAVObject ));
}

ERROR_RETURN int8_t respondNacknowledge( uint16_t instanceID )
{
    UAVObject obj;
    createNACK(&obj, instanceID );
    
    return GD_sendMessage( instance ,&obj , sizeof( UAVObject ));
}


ERROR_RETURN ALWAYS_INLINE int tryLockThread( void )
{
    return GD_tryLockDispatch( instance );
}
ERROR_RETURN ALWAYS_INLINE int unlockThread( void )
{
    return GD_unlockDispatch( instance );
}


/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

int8_t dispatchMainLoop(void *data)
{
    GrandDispatcher *dispatch = (GrandDispatcher*) data;
    
    if( IPC_connectToServer( &dispatch->_thread._port ) != IPC_noerror)
        return IPC_connect;
    
    const pid_t pid = getpid();
    
    /**/
    
    dispatch->state = 1;
    
    Message_buf  rbuf;
    Message_buf  sbuf;
    
    /* Send registration request */
    
    sbuf.mtype = IPC_ProcessRegistration;
    sbuf.pid = pid;
    
    
    if( IPC_send(&dispatch->_thread._port, &sbuf, sizeof(Message_buf))<=0)
    {
        printf("Error send IPC_ProcessRegistration \n");
    }
    
    /* Wait for reply ... */
    
    uint8_t didReply = 0;
    uint32_t counter = 0;
    const uint32_t maxTime = 40;
    
    ssize_t t = 0;
    
    while ( counter < maxTime )
    {
        if( IPC_selectRead( &dispatch->_thread._port ) == IPC_noerror )
        {
            if ((t = IPC_receive( &dispatch->_thread._port, &rbuf, sizeof(Message_buf) ) > 0))
            {
                if ( rbuf.mtype == IPC_ProcessDidRegister)
                {
                    didReply = 1;
                    break;
                }
            }
        }
        
        counter++;
        
    }
    
    if( didReply == 0)
    {
        dispatch->_callBack1( ConnectionError ,NULL,  dispatch->_callBackUserData1 );
        
        return IPC_refused;
    }
    else // we're good!
    {
        
        
        dispatch->state = 2;
        
        dispatch->_callBack1( DidRegisterToDispatcher ,NULL, dispatch->_callBackUserData1 );
        
        /* Send an internal info request */
        
        sbuf.mtype = IPC_PrivateRequest;
        IPC_send( &dispatch->_thread._port , &sbuf, sizeof(Message_buf ));
        
        
        while ( dispatch->_thread.shouldQuit == 0 )
        {
            
            const int8_t ret = IPC_selectRead( &dispatch->_thread._port);
            if(  ret == IPC_noerror )
            {
                if ((t = IPC_receive(&dispatch->_thread._port, &rbuf, sizeof(Message_buf ))> 0))
                {
                    
                    if( dispatch->_callBack1)
                    {
                        
                        if( rbuf.mtype == IPC_DataSend )
                        {
                            dispatch->_callBack1( DidReceiveData ,&rbuf, dispatch->_callBackUserData1 );
                        }
                        
                        else if(rbuf.mtype >= IPC_PrivateRequest )
                        {
                            dispatch->_callBack1( (int) rbuf.mtype ,&rbuf, dispatch->_callBackUserData1 );
                        }
                        
                    }
                    
                    
                }
                else
                {
                    
                    printf("Read error, quit! %i \n" , dispatch->_thread._port.lastReceiveError);
                    GD_stop( dispatch );
                    
                }
            }
            else if( ret == IPC_timeout)
            {
                if( dispatch->threadedLoop == 0)
                {
                    if( dispatch->_userTaskCallBack != NULL)
                        dispatch->_userTaskCallBack(dispatch->_userTaskData );
                }
            }
            
        } // end of while
        
        dispatch->_callBack1( WillTerminateConnection , NULL , dispatch->_callBackUserData1 );
        
        sbuf.pid = pid;
        
        sbuf.mtype = IPC_ProcessDeregistration;
        if( IPC_send(&dispatch->_thread._port, &sbuf, sizeof(Message_buf)) <= 0)
        {
        }
    }
    


    return IPC_noerror;
}

/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

void eventReceived( int reason, const void* msg, void* data)
{
    
    if( reason < DidReceiveData )
    {
        if( reason == DidRegisterToDispatcher)
            _connected = 1;
        
        else if( (reason == Connection_Error) || ( reason == WillTerminateConnection ) )
            _connected = 0;
        
        if( params.notificationsCallBack )
            params.notificationsCallBack( reason , data );
    }
    else if (reason == DidReceiveData)
    {
        if( params.parseObjectsCallBack )
        {
            UAVObject obj;
            initUAVObject( &obj );

            memcpy(&obj, ((const Message_buf *) msg)->data.buffer, sizeof( UAVObject ));

            params.parseObjectsCallBack(&obj , data);

        }
    }
    else if( reason == IPC_PingRequest )
    {
        Message_buf buff;
        buff.mtype = IPC_PingResponse;
        
        /*
        static int c = 0;
         c++;
        if( c < 3 || c > 6)*/
        IPC_send( &instance->_thread._port, &buff, sizeof(Message_buf ));


    }
    else if( reason == IPC_PrivateRequestResponse )
    {
        const Message_buf *message = (const Message_buf*) msg;
        
        _runtimeInfos.plateform = message->data.buffer[ offsetof(RuntimeInformations, plateform ) ];
        
        memcpy(_runtimeInfos.name, message->data.buffer+offsetof(RuntimeInformations, name), NAME_MAX_SIZE );
        memcpy(_runtimeInfos.constructor, message->data.buffer+offsetof(RuntimeInformations, constructor), NAME_MAX_SIZE );
        
        _runtimeInfos.versionMin = message->data.buffer[ offsetof(RuntimeInformations, versionMin ) ];
        _runtimeInfos.versionMaj = message->data.buffer[ offsetof(RuntimeInformations, versionMaj ) ];
        
        _runtimeInfos.hardwareStatus = (int8_t) message->data.buffer[ offsetof(RuntimeInformations, hardwareStatus)];
        
        if( params.notificationsCallBack )
            params.notificationsCallBack( InformationsAvailable , data );
    }
    else
    {
        printf("Other Reason %i" , reason);
    }

}
