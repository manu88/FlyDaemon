#include <stdio.h>
#include <stdlib.h>


#include <unistd.h>
#include "../../libDaemon/FlyLabAPI.h"


uint8_t connected = 0;


void printUAVObject( const UAVObject *obj )
{
    /*
     obj->sync       = 0;
     obj->type       = 0;
     obj->length     = 0;
     obj->objectID   = 0;
     obj->instanceID = 0;
     obj->timestamp  = 0;
     obj->checksum   = 0;*/
    printf("***************\n");
    printf("sync %i \n" , obj->sync );
    printf("type %i \n" , obj->type );
    printf("length %i \n" , obj->length );
    printf("objectID %i \n" , obj->objectID );
    printf("instanceID %i \n" , obj->instanceID );
    printf("timestamp %i \n" , obj->timestamp );
    printf("checksum %i \n" , obj->checksum );
    printf("data '%s' \n" , (char*)obj->data );
}

static void onNotification(int errorNum, void *userData)
{

    
    if( errorNum == Connection_Error )
    {
        printf("ConnectionError \n");
    }
    else if( errorNum == Connection_WillEnd )
    {
        printf("WillTerminateConnection \n");
    }
    else if( errorNum == Connection_OK )
    {
        printf("IsConnected \n");
        
        connected = 1;
    }
    else
    {
        printf("Notification received %i\n" , errorNum );
    }
}

static void uavObjectReceived( const UAVObject *obj , void* userData )
{
    printUAVObject( obj);
}

int main (void)
{
    FlyLabParameters params;
    params.function = uavObjectReceived;
    params.notificationsCallBack = onNotification;
    
    initializeConnection( &params );
    
    uint32_t objectID = 0;
    if( runFromNewThread() == 1)
    {
        while (connected == 0)
        {
            usleep( 1000 );
        }
        while ( isConnected() )
        {
            sendObjectRequest( objectID++ );
            usleep( 100000 );
        }
        printf("thread ended\n");
    }
    else
        printf("Error creation thread\n");
    
    
    cleanup();
    
    return EXIT_SUCCESS;
}
