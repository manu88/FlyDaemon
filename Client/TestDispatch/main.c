#include <stdio.h>
#include <stdlib.h>


#include <unistd.h>
#include "../../libDaemon/FlyLabAPI.h"


uint8_t connected = 0;

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
    printf("UAV object received \n");
}
int main (void)
{
    FlyLabParameters params;
    params.function = uavObjectReceived;
    params.notificationsCallBack = onNotification;
    
    initializeConnection( &params );
    
    if( runFromNewThread() == 1)
    {

        while ( isConnected() )
        {

            sendObjectRequest( 1);
        }
        printf("thread ended\n");
    }
    else
        printf("Error creation thread\n");
    
    
    cleanup();
    
    return EXIT_SUCCESS;
}
