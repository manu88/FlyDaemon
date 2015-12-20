#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "../../libDaemon/src/Dispatch.h"


void onData(int reason, const void* msg, void *userdata)
{
    static int dataCounter = 0;
    
    if( reason == DidRegisterToDispatcher)
    {
        printf("Did register \n");
    }
    else if( reason == DidReceiveData)
    {
        printf("DidReceiveData \n");
        if( dataCounter++ > 1000)
        {
            GrandDispatcher* dispatch = (GrandDispatcher*) userdata;
            GD_stop(dispatch);
        }
    }
    else if( reason == WillTerminateConnection)
    {
        printf("WillTerminateConnection\n");
    }
    else
        printf(" Did receive data %i \n" , reason );
    
}


int main(void)
{
    GrandDispatcher *dispatch = GD_init();

    dispatch->_callBack1 = onData;
    dispatch->_callBackUserData1 = dispatch;
    
    GD_runFromLoop( dispatch );
    printf("After main Loop\n");
    GD_release( dispatch );
    return 0;
}
/*
#include "../../PrivateAPI/IPCComm.h"


int main(void)
{
    char str[100];
     ssize_t t = 0;
    
    IPCCommunicationPort port;
    assert( IPC_initialize( &port)== IPC_noerror );
    assert( IPC_createClient( &port)== IPC_noerror );

    if(IPC_connectToServer( &port ) == IPC_noerror )
    {
        if( port.connected == 1)
            printf("Connected.\n");

        uint8_t count = 0;
        while( count < 100)
        {
            str[1] = count++;


            
            if ( IPC_send(&port, str, strlen(str)) == -1)
            {
                perror("send");
                exit(1);
            }
      
            if ((t = IPC_receive(&port, str, 100)> 0))
            {
                uint8_t val = (uint8_t)str[1];
                printf("\necho> %i", val);
            }
            else
            {
                if (t < 0) perror("recv");
                else printf("Server closed connection\n");
                exit(1);
            }

            

        }
        
    }
        else
            printf("Error Connection.\n");
    
    IPC_closeClient( &port);
    printf("Close Client \n");    
    return 0;
}
*/
/*
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
 */
