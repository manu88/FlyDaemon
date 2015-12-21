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
#include <stdlib.h> // pour free() : pas bien!
#include <stddef.h>
#include <stdio.h>
#include "../include/FlyLabAPI.h"
#include "Dispatch.h"
#include "IPCMessage.h"


static const char _version[] = "0.0.1";

static RuntimeInformations _runtimeInfos;
static GrandDispatcher *instance = NULL;
static FlyLabParameters params;

static uint8_t _connected = 0;

void eventReceived( int reason, const void* msg, void* data);

uint8_t initializeConnection( const FlyLabParameters *parameters )
{
    
    memset( &_runtimeInfos, 0, sizeof(RuntimeInformations ));
    
    params = *parameters;
    
    instance = GD_init();
    
    _connected = 0;
    
    GD_setCallBack1( instance, eventReceived, parameters->userData );

    return instance != NULL;
}

void cleanup()
{
    if( instance->state > 0)
        GD_stop( instance);
    
    if (instance != NULL)
        GD_release( instance );
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

ALWAYS_INLINE uint8_t disconnect()
{
    return GD_stop( instance );
}

uint8_t runFromThisThread()
{
    return GD_runFromLoop( instance );
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

int8_t respondAcknowledge( uint16_t instanceID )
{
    UAVObject obj;
    createACK(&obj, instanceID );
    
    return GD_sendMessage( instance ,&obj , sizeof( UAVObject ));
}

int8_t respondNacknowledge( uint16_t instanceID )
{
    UAVObject obj;
    createNACK(&obj, instanceID );
    
    return GD_sendMessage( instance ,&obj , sizeof( UAVObject ));
}


ALWAYS_INLINE int lockThread( void )
{
    return GD_lockDispatch( instance );
}
ALWAYS_INLINE int unlockThread( void )
{
    return GD_unlockDispatch( instance );
}


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
//            parseIPC( msg, &obj);
            memcpy(&obj, ((const Message_buf *) msg)->data.buffer, sizeof( UAVObject ));

            params.parseObjectsCallBack(&obj , data);

        }
    }
    else if( reason == IPC_PingRequest )
    {

        printf("received ping request \n");
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
        //
        
        if( params.notificationsCallBack )
            params.notificationsCallBack( InformationsAvailable , data );
    }
    else
    {
        printf("Other Reason %i" , reason);
    }

}
