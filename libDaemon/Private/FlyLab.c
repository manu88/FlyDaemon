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


static GrandDispatcher *instance = NULL;

static FlyLabParameters params;

void eventReceived( int reason, void* data);

uint8_t initializeConnection( const FlyLabParameters *parameters )
{
    
    params = *parameters;
    
    instance = GD_init();
    
    GD_setCallBack1( instance, eventReceived, NULL );

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

uint8_t sendObject( const UAVObject * obj)
{
    return 1;
}

uint8_t sendObjectRequest( uint32_t objectID)
{
    
    const char test[] = "hello";
    return GD_sendMessage( instance ,(void*)test , 6) ;

}




void eventReceived( int reason, void* data)
{
    
    static int count = 0;
    
    if( reason < DidReceiveData )
    {
        if( params.notificationsCallBack )
            params.notificationsCallBack( reason , params.userData );
    }
    else if (reason == DidReceiveData)
    {
        if( params.function )
            params.function(NULL , params.userData);
    }
    /*
    if( reason == DidRegisterToDispatcher )
    {
        printf("DidRegisterToDispatcher \n");
    }
    
    else if( reason == ConnectionError )
    {
        printf("Connection error quit\n");
        if( params.notificationsCallBack )
            params.notificationsCallBack( reason , params.userData );
    }
    
    else if( reason == DidReceiveData )
    {
        if( params.function )
            params.function(NULL , params.userData);
    }
     */
    else
    {
        printf("Other Reason %i" , reason);
    }
    count++;
    
    
    if( count > 100)
    {
        GD_stop( instance );
    }
}
