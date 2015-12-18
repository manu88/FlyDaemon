#include <stdio.h>
#include <stdlib.h>


#include <unistd.h>
#include "../../libDaemon/FlyLabAPI.h"


uint8_t connected = 0;


void printUAVObject( const UAVObject *obj )
{

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
        connected = 0;
    }
    else if( errorNum == Connection_WillEnd )
    {
        printf("WillTerminateConnection \n");
        connected = 0;
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
    static int count = 0;
    static uint32_t lastID = 0;
//    printUAVObject( obj);
  
    if( (obj->objectID - 1) != lastID )
        printf("Error Counting at %i (last is %i ) \n" , obj->objectID , lastID );
    
    lastID = obj->objectID;

    if( count++ > 1000)
        disconnect();
}

int main (void)
{
    FlyLabParameters params;
    params.parseObjectsCallBack = uavObjectReceived;
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
            usleep( 1000 );
        }
        printf("thread ended\n");
    }
    else
        printf("Error creation thread\n");
    
    
    cleanup();
    
    return EXIT_SUCCESS;
}
