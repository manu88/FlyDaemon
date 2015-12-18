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
#include "../FlyLabAPI.h"
#include "Dispatch.h"
#include "IPCMessage.h"

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

void cleanup(void)
{
    if( instance->state > 0)
        GD_stop( instance);
    
    if (instance != NULL)
        GD_release( instance );
}



uint8_t disconnect( void )
{
    return GD_stop( instance );
}

uint8_t runFromThisThread( void )
{
    return GD_runFromLoop( instance );
}
uint8_t runFromNewThread(void)
{
    if( GD_runFromThread( instance )  == 0 )
        return 0;
    
    if( GD_waitForCreation( instance ) == 0 )
        return 0;
        
    return instance->state >= 1;
}

uint8_t isConnected( void )
{
    return instance->state >= 1;
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
            parseIPC( msg, &obj);
            params.parseObjectsCallBack(&obj , data);
        }
    }
    else
    {
        printf("Other Reason %i" , reason);
    }

}
