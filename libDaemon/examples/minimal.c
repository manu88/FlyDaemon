/**
 * \file minimal.c
 * \brief API minimalistic example
 * \author Manuel Deneu
 * \version 0.1
 * \date 22/12/2015
 *
 * This examples shows a minimal skeleton for your code.
 *
 */
#include <stdio.h>
#include <unistd.h>

#include "../../libDaemon/include/FlyLabAPI.h"

void printRuntimeInformations( void );
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

/*
    This function will be called when a notification is received
 */
static void onNotification(int errorNum, void *userData)
{
    UNUSED_PARAMETER(userData);
    
    if( errorNum == Connection_Error )
    {
        printf("Connection error \n");
    }
    
    else if( errorNum == Connection_WillEnd )
    {
        printf("Connection will end \n");
    }
    
    else if( errorNum == Connection_OK )
    {
        printf("Connection is ok \n");
    }
    
    else if( errorNum == InformationsAvailable )
    {
        printRuntimeInformations();
    }

}

void printRuntimeInformations()
{
    const RuntimeInformations *infos = getRuntimeInformations();
    
    printf("Infos Plateform ver %i.%i : %s \n" , infos->versionMin , infos->versionMaj, infos->plateform == Plateform_Drone?"Drone" : "Simulator");
    printf("           Name : %s \n" , infos->name );
    printf("    constructor : %s \n" , infos->constructor );
    printf("hardware status : %i\n",infos->hardwareStatus );
}

static void uavObjectReceived( const UAVObject *obj , void* userData )
{
    UNUSED_PARAMETER(userData); 
    
    static uint32_t receivedID = 0;
    printUAVObject( obj);
    
    if( obj->objectID == receivedID )
    {
        receivedID++;
    }
    else
    {
        printf(" Error expected %i got %i\n" , receivedID , obj->objectID );
        receivedID = obj->objectID;
        
    }
    
    if( obj->type == Type_OBJ_ACK )
    {
        respondAcknowledge( obj->instanceID );
        respondNacknowledge( obj->instanceID );
    }
}

int main (int argc, char *argv[])
{
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(argv);
    
    printf(" Flylab API ver %s\n" , API_getVersion() );
    
    FlyLabParameters params;
    params.parseObjectsCallBack = uavObjectReceived;
    params.notificationsCallBack = onNotification;
    
    initializeConnection( &params );

    if( runFromNewThread() == 0)
    {
        printf("Error creation thread with code %i \n" ,getReturnValue());
    }
    else
    {
        
        while ( isConnected() != 1)
        {
            usleep( 1000 );
        }
        
        
        uint32_t objectID = 0;
        
        while ( isConnected() )
        {
            sendObjectRequest( objectID++ ) ;

            usleep( 500 );
            
            if( objectID > 1000 )
            {
                //Send Quit signal
                disconnect();
            }
            
        }
    }
    
    cleanup();
    
    return 0;
}

