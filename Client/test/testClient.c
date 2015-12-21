
#include <stdio.h>
#include <unistd.h>
#include "../../libDaemon/include/FlyLabAPI.h"


#define UNUSED_PARAMETER(x) (void)(x)


void printInformations( void );
void printUAVObject( const UAVObject *obj );


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
    UNUSED_PARAMETER(userData);
    
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
        

    }
    else if( errorNum == InformationsAvailable )
    {
        printInformations();
    }
    else
    {
        printf("Notification received %i\n" , errorNum );
    }
}

void printInformations()
{
    const RuntimeInformations *infos = getRuntimeInformations();
    
    printf("Infos Plateform ver %i.%i : %s \n" , infos->versionMin , infos->versionMaj, infos->plateform == Plateform_Drone?"Drone" : "Simulator");
    printf("           Name : %s \n" , infos->name );
    printf("    constructor : %s \n" , infos->constructor );
}

static void uavObjectReceived( const UAVObject *obj , void* userData )
{
    UNUSED_PARAMETER(userData);
    
    static long count = 0;
    
    
    if( obj->type == Type_OBJ_ACK )
    {
        printf("respondAcknowledge %i %li \n", obj->instanceID , count);
        respondAcknowledge( obj->instanceID );
    }
    //printUAVObject( obj);

    count++;
    if( (count % 50 ) == 0)
        printf(" received count %li \n" , count);
    
    if( count > 500000 )
        disconnect();

}

int main (void)
{
    printf(" Flylab API ver %s\n" , API_getVersion() );
    
    FlyLabParameters params;
    params.parseObjectsCallBack = uavObjectReceived;
    params.notificationsCallBack = onNotification;
    
    initializeConnection( &params );
    
    uint32_t objectID = 0;
    if( runFromNewThread() == 1)
    {
        while ( isConnected() != 1)
        {
            usleep( 1000 );
            printf("Wait for connection ... \n");
        }
        while ( isConnected() )
        {
            sendObjectRequest( objectID++ );
            usleep( 100 );
        }
        printf("thread ended\n");
    }
    else
        printf("Error creation thread\n");
    
    
    cleanup();
    
    return 0;
}

