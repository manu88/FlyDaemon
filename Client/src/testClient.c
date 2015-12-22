
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h> // memset
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



static long lock_errors = 0;

static void uavObjectReceived( const UAVObject *obj , void* userData )
{
    UNUSED_PARAMETER(userData);
    

    printUAVObject( obj);
    
    
    if( obj->type == Type_OBJ_ACK )
    {
        if (obj->instanceID % 2)
            respondAcknowledge( obj->instanceID );
        else
            respondNacknowledge( obj->instanceID );
    }

}

int main (void)
{
    printf(" Flylab API ver %s config %s \n" , API_getVersion() , isInDebugMode()?"DEBUG" : "RELEASE" );
    
    FlyLabParameters params;
    params.parseObjectsCallBack = uavObjectReceived;
    params.notificationsCallBack = onNotification;
    
    initializeConnection( &params );


    
    char word[256];
    memset(word , 0 , 256);
    
    if( runFromNewThread() == 1)
    {

        printf("Wait for connection ... \n");
        while ( isConnected() != 1)
        {
            usleep( 1000 );
        }
        printf("Connected \n");
        while ( isConnected() )
        {
            printf("\n UAV request > ");
            fgets(word, sizeof(word), stdin);
            strtok(word, "\n");
            
            
            if( strcmp(word, "quit") == 0 )
                disconnect();
            else if( strcmp(word, "infos") == 0 )
            {
                requestRuntimeInformationsUpdate();
            }
            else
            {
                const int req = atoi(word);
                if( req > 0)
                {
                    const int8_t ret= sendObjectRequest((uint32_t ) req ) ;
                    if(ret == -2)
                    {
                        lock_errors++;
                    }
                }
            }


            
        }
        printf("thread ended  \n");
        

    }
    else
        printf("Error creation thread with code %i \n" ,getReturnValue());
    
    
    cleanup();
    
    return 0;
}

