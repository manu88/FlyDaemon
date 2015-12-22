#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

#include "../../PrivateAPI/IPCComm.h"
#include "../../libDaemon/src/IPCMessage.h"
#include "../../libDaemon/include/FlyLabAPI.h"

#include "../../libDaemon/src/Dispatch.h"

/* **** **** **** **** **** **** **** **** **** **** **** **** **** */
/* Parameters */

const uint8_t plateformType = Plateform_Simulator;

const char    plateformName[] = "My drone";
const char    constructor[] = "FlyLab inc.";

const uint8_t minVer = 01;
const uint8_t majVer = 10;

const uint8_t maxPendingPing = 10;
const unsigned long deltaPing = 1000;

uint32_t errorsCounter = 0;

int8_t hardwareStatus = -1; /* unknown */
/* **** **** **** **** **** **** **** **** **** **** **** **** **** */

static uint32_t id = 0;


static volatile int keepRunning = 1;
static volatile int done = 0;

uint8_t pingCount = 0;


IPCCommunicationPort port;

void initList( void );
int findPid( const pid_t pid );
int removePid( const pid_t pid );
int addPid( const pid_t pid );
uint8_t getNumClients(void);
void printList( void );
void receive(void*data, ssize_t size);

#define MAX_CLIENTS 10

pid_t registeredPIDs[ MAX_CLIENTS ];

void initList( void )
{
    for (uint8_t i = 0; i < MAX_CLIENTS ; i++)
        registeredPIDs[i] = 0;
}

int findPid( const pid_t pid )
{
    for (uint8_t i = 0; i < MAX_CLIENTS ; i++)
    {
        if (registeredPIDs[i] == pid )
        {
            return i;
        }
    }
    return -1;
}

int removePid( const pid_t pid )
{
    for (uint8_t i = 0; i < MAX_CLIENTS ; i++)
    {
        if (registeredPIDs[i] == pid )
        {
            registeredPIDs[i] = 0;
            return i;
        }
    }
    
    return -1;
}

int addPid( const pid_t pid )
{
    if( findPid(pid) == -1 )
    {
        for (uint8_t i = 0; i < MAX_CLIENTS ; i++)
        {
            if( registeredPIDs[i] == 0 )
            {
                registeredPIDs[i] = pid;
                return i;
            }
        }
        return -2; // no slots available MAX_CLIENTS reached
    }
    
    return -1;
}

uint8_t getNumClients(void)
{
    uint8_t count = 0;
    
    for (uint8_t i = 0; i < MAX_CLIENTS ; i++)
    {
        if( registeredPIDs[i] != 0 )
            count++;
        
    }
    
    return count;
}

void printList()
{
    for (uint8_t i = 0; i < MAX_CLIENTS ; i++)
    {
        printf("SLOT %i pid = %i \n" , i , registeredPIDs[i] );
    }
}

/* **** **** **** **** **** **** **** **** **** **** **** **** **** */

void receive(void*data, ssize_t size)
{
    if( size > ( (ssize_t) sizeof(Message_buf ) ) )
    {
        printf("Message size is %zi - expected %lu\n" , size , sizeof(Message_buf ));
        assert( 0 );
    }
    const Message_buf* in = ( const Message_buf*) data;
    
    if( in->mtype == IPC_ProcessRegistration)
    {
        printf("Received IPC_ProcessRegistration from %i \n", in->pid);

        addPid( in->pid );
        printList();
        Message_buf out;
        out.mtype = IPC_ProcessDidRegister;
        
        

        if( IPC_selectWrite(  &port ) == IPC_noerror)
        if ( IPC_send(&port, &out, sizeof(Message_buf)) < 0)
        {
            perror("send");
        }
    }
    else if( in->mtype == IPC_ProcessDeregistration)
    {
        printf("Received IPC_ProcessDeregistration from %i\n", in->pid);
        
        removePid( in->pid);
        printList();
    }
    else if( in->mtype == IPC_PrivateRequest )
    {
        printf("IPC_PrivateRequest \n");
        
        Message_buf outBuffer;
        outBuffer.mtype = IPC_PrivateRequestResponse;
        
        
        outBuffer.data.buffer[  offsetof(RuntimeInformations, plateform) ] = plateformType;
        
//        strcpy((char*)outBuffer.data.buffer+2, plateformName);
        memcpy(outBuffer.data.buffer+ offsetof(RuntimeInformations, name), plateformName, NAME_MAX_SIZE );
        memcpy(outBuffer.data.buffer+ offsetof(RuntimeInformations, constructor), constructor, NAME_MAX_SIZE );
        
        
        outBuffer.data.buffer[ offsetof(RuntimeInformations, versionMin )] = minVer;
        outBuffer.data.buffer[ offsetof(RuntimeInformations, versionMaj )] = majVer;
        
        outBuffer.data.buffer[ offsetof(RuntimeInformations, hardwareStatus)] = (uint8_t) hardwareStatus;
        
        
        if( IPC_selectWrite(  &port ) == IPC_noerror)
        if ( IPC_send(&port, &outBuffer, sizeof(Message_buf)) < 0)
        {
            perror("send");
        }
        
    }
    else if( in->mtype == IPC_PingResponse )
    {
        printf("Ping response  last count %i\n" , pingCount );
        
        pingCount = 0;
    }
    else if( in->mtype == IPC_DataRequest )
    {
        const UAVObject* inObj = (const UAVObject*) in->data.buffer;
        
        if( findPid( in->pid) != -1)
        {
            if( inObj->type == Type_ACK)
            {
//                printf("Received acknowledge for %i \n" , inObj->instanceID );
            }

            

            
            if( inObj->type == Type_OBJ_REQ)
            {
                
                Message_buf outBuffer;
                UAVObject outObject;

                outBuffer.mtype = IPC_DataSend;
                dumbUAVObject(&outObject);
                outObject.type = Type_OBJ_ACK;
                outObject.objectID = inObj->objectID;
                
                if( inObj->objectID != id)
                {
                    printf("objectID mismatch got %i  expected %i \n", inObj->objectID , id);
                    errorsCounter++;
                    id = inObj->objectID;
                }
                id++;
                strcpy((char*)outObject.data, "response");
                memcpy(outBuffer.data.buffer , &outObject , sizeof(UAVObject) );
                
//                printf("Received Object request for %i \n" , inObj->objectID );
                
                if( IPC_selectWrite(  &port ) == IPC_noerror)
                if ( IPC_send(&port, &outBuffer, sizeof(Message_buf)) < 0)
                {
                    perror("send");
                }
            }


            
        }
    }

}



int main(void)
{

    initList();

    Message_buf inBuffer;
    Message_buf outBuffer;
    
    UAVObject outObject;
    
    int err = 0;

    assert( IPC_initialize( &port) == IPC_noerror );
    assert( IPC_createServer( &port )== IPC_noerror );


    while( keepRunning )
    {
        id = 0;
        
        struct timeval tv;
        tv.tv_sec =  60;
        tv.tv_usec = 0;
        printf("Wait for a connection \n");
        const int8_t ret = IPC_waitForClient( &port , &tv) ;
        
        if( ret == IPC_noerror )
            printf("Connected.\n");
        else if( ret == IPC_timeout )
        {
            printf("Wait timeout \n");
            continue;
        }
        else
            printf("Error IPC_waitForClient \n");

        ssize_t n = 0;
        
        
        clock_t last = clock();
        
        pingCount = 0;
        
        errorsCounter = 0;
        
        do
        {
            
//            time = clock();
            
            const unsigned long diffMS = (clock() -last )* 1000 / CLOCKS_PER_SEC;

            
            if( diffMS > deltaPing   )
            {
                last = clock();

                outBuffer.mtype = IPC_PingRequest;
                
                if( IPC_selectWrite(  &port ) == IPC_noerror)
                if( IPC_send( &port , &outBuffer, sizeof(Message_buf))<=0)
                {
                    printf("Error send IPC_ProcessRegistration \n");
                }
                
                pingCount++;
                
                if( pingCount >= maxPendingPing)
                {
                    printf("Client not responding \n");
                    break;
                }
            }

            
            
            const int8_t retSelect = IPC_selectRead( &port);
            if( retSelect == IPC_noerror )
            {
                n = IPC_receive(&port, &inBuffer, sizeof(Message_buf));
                err = port.lastReceiveError;
                if( n>0)
                {
                    receive( &inBuffer, n);
                    
                    memset(&inBuffer,0, 100);
                    
                    
                }
                else if (n <= 0)
                {
                    if( err == ETIMEDOUT)
                    {
                        printf("Read timeout\n");
                        
                        break;

                    }
                    else if( err != EAGAIN)
                    {
                        printf("recv returned unrecoverable error(errno=%d)\n", err);
                        break;
                    }
                }
            }

            else if( retSelect == IPC_timeout )
            {
                /*
                outBuffer.mtype = IPC_DataSend;
                dumbUAVObject(&outObject);
                outObject.type = Type_OBJ_ACK;
                outObject.instanceID = 42;
                memcpy(outBuffer.data.buffer , &outObject , sizeof(UAVObject) );

                if( IPC_selectWrite(  &port ) == IPC_noerror)
                if ( IPC_send(&port, &outBuffer, sizeof(Message_buf)) < 0)
                {
                    perror("send");
                }
                
                if(port.lastSendError == EPIPE )
                {
                    printf("PIPE Send error \n");
                    break;
                }
                 */
            }

        } while (!done);
        
        IPC_closeServer( &port );
        printf("Connection closed\n");
        printf("Got %i errors on the session\n",errorsCounter);
        
        initList();
        done = 0;
    }

    printf("Close Server \n");
    
    return 0;
}
