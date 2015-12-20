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

#include <stdio.h>
#include "../include/FlyLabAPI.h"
#include "Dispatch.h"
#include "IPCMessage.h"


static const char _version[] = "0.0.1";

static uint8_t _plateform = Plateform_Unknown;

static GrandDispatcher *instance = NULL;

static FlyLabParameters params;

void eventReceived( int reason, const void* msg, void* data);

uint8_t initializeConnection( const FlyLabParameters *parameters )
{
    
    params = *parameters;
    
    instance = GD_init();
    
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

const char* API_getVersion()
{
    return _version;
}

uint8_t informationsAvailable()
{
    return _plateform != Plateform_Unknown;
}

uint8_t getRuntimeInformations( RuntimeInformations* infos)
{
    if( infos == NULL)
        return 0;
    
    if( !informationsAvailable())
        return 0;
    
    infos->plateform = _plateform;
    
    return 1;
}

uint8_t disconnect()
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

uint8_t isConnected()
{
    return instance->state > 1;
}

int8_t sendObject( const UAVObject * obj)
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


void eventReceived( int reason, const void* msg, void* data)
{
    
    if( reason < DidReceiveData )
    {
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
    else if( reason == PrivateInformationsUpdated )
    {

        const Message_buf *message = (const Message_buf*) msg;
        _plateform = message->data.buffer[0];

        printf("PrivateInformationsUpdated : Plateform %s \n" , _plateform == Plateform_Drone?"Drone" : "Simulator");
    }
    else
    {
        printf("Other Reason %i" , reason);
    }

}
