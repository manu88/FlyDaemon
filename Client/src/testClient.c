
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "../../libDaemon/include/FlyLabAPI.h"

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
    printf("hardware status : %i\n",infos->hardwareStatus );
}


static long rec_count = 0;
static long lock_errors = 0;

static void uavObjectReceived( const UAVObject *obj , void* userData )
{
    UNUSED_PARAMETER(userData);
    
    static uint32_t receivedID = 0;
//    printUAVObject( obj);
    
    if( obj->objectID == receivedID )
    {
        receivedID++;
    }
    else
    {
        printf(" Error expected %i got %i\n" , receivedID , obj->objectID );
        receivedID = obj->objectID;
        rec_count++;
    }
    
    if( obj->type == Type_OBJ_ACK )
    {
        respondAcknowledge( obj->instanceID );
        respondNacknowledge( obj->instanceID );
    }

}

int main (int argc, char *argv[])
{

    clock_t begin, end;
    double time_spent;
    
    unsigned int usDT = 1000;
    int nbIter = 10000;
    
    if( argc >= 3)
    {
        usDT   = (unsigned int) atoi( argv[1] );
        nbIter = atoi( argv[2] );
    }
    printf("Start test with DT = %i COUNT = %i\n" , usDT , nbIter);
    printf(" Flylab API ver %s config %s \n" , API_getVersion() , isInDebugMode()?"DEBUG" : "RELEASE" );
    
    FlyLabParameters params;
    params.parseObjectsCallBack = uavObjectReceived;
    params.notificationsCallBack = onNotification;
    
    initializeConnection( &params );

    uint32_t objectID = 0;
    
    long count = 0;
    if( runFromNewThread() == 1)
    {
        begin = clock();
        printf("Wait for connection ... \n");
        while ( isConnected() != 1)
        {
            usleep( 1000 );
        }
        printf("Connected \n");
        while ( isConnected() )
        {
            
            const int8_t ret= sendObjectRequest( objectID ) ;
            if(ret == -2)
            {
                lock_errors++;
            }
            else if( ret > 0)
                objectID++;
                
            usleep( usDT );
            count++;
            
            if( count > nbIter )
            {
                printf("Send Quit signal \n");
                disconnect();
            }
            
        }
        printf("thread ended  sent Count = %li , Errors %li\n" , count ,rec_count);
        
        end = clock();
        time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        
        printf("Spent %f seconds \n" , time_spent);
        printf(" Failed Locks %li \n" , lock_errors);
        
        
    }
    else
        printf("Error creation thread with code %i \n" ,getReturnValue());
    
    
    cleanup();
    
    return 0;
}

