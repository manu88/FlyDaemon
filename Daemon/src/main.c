#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include "../../PrivateAPI/IPCComm.h"
#include "../../libDaemon/src/IPCMessage.h"
#include "../../libDaemon/include/FlyLabAPI.h"

#define UNUSED_PARAMETER(x) (void)(x)

/* **** **** **** **** **** **** **** **** **** **** **** **** **** */
/* Parameters */

const uint8_t plateformType = Plateform_Simulator;

const char    plateformName[NAME_MAX_SIZE] = "My drone";
const char    constructor[NAME_MAX_SIZE] = "FlyLab inc.";

const uint8_t minVer = 01;
const uint8_t majVer = 10;
/* **** **** **** **** **** **** **** **** **** **** **** **** **** */

static volatile int keepRunning = 1;
static volatile int done = 0;
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
        
        if ( IPC_send(&port, &outBuffer, sizeof(Message_buf)) < 0)
        {
            perror("send");
        }
        
    }
    else if( in->mtype == IPC_DataRequest )
    {
//        printf("Received IPC_DataRequest from %i\n", in->pid);
        
        const UAVObject* inObj = (const UAVObject*) in->data.buffer;
        
        if( findPid( in->pid) != -1)
        {
            if( inObj->type == Type_ACK)
                printf("Received acknowledge for %i \n" , inObj->instanceID );
            

            

            
            if( inObj->type == Type_OBJ_REQ)
            {
                Message_buf outBuffer;
                UAVObject outObject;
                dumbUAVObject( &outObject );
                outBuffer.mtype = IPC_DataSend;
                dumbUAVObject(&outObject);
                outObject.type = Type_OBJ_ACK;
                
                strcpy((char*)outObject.data, "response");
                memcpy(outBuffer.data.buffer , &outObject , sizeof(UAVObject) );
                
                printf("Received Object request for %i \n" , inObj->objectID );
                
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

        struct timeval tv;
        tv.tv_sec =  3;
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

        do
        {
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
                outBuffer.mtype = IPC_DataSend;
                dumbUAVObject(&outObject);
                outObject.type = Type_OBJ_ACK;
                outObject.instanceID = 42;
                memcpy(outBuffer.data.buffer , &outObject , sizeof(UAVObject) );

                if ( IPC_send(&port, &outBuffer, sizeof(Message_buf)) < 0)
                {
                    perror("send");
                }
                
                if(port.lastSendError == EPIPE )
                {
                    printf("PIPE Send error \n");
                    break;
                }
            }

        } while (!done);
        
        IPC_closeServer( &port );
        printf("Connection closed\n");
        
        initList();
        done = 0;
    }

    printf("Close Server \n");
    
    return 0;
}
