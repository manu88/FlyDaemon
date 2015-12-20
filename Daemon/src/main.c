
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#include "../../PrivateAPI/IPCComm.h"
#include "../../libDaemon/src/IPCMessage.h"
#include "../../libDaemon/include/FlyLabAPI.h"


/* **** **** **** **** **** **** **** **** **** **** **** **** **** */
/* Parameters */

const uint8_t plateformType = Plateform_Simulator;

const char    plateformName[NAME_MAX_SIZE] = "awsomeDroneName";
const char    constructor[NAME_MAX_SIZE] = "FlyLab inc.";

const uint8_t minVer = 01;
const uint8_t majVer = 10;
/* **** **** **** **** **** **** **** **** **** **** **** **** **** */

static volatile int keepRunning = 1;
static volatile int done = 0;
IPCCommunicationPort port;

/* **** **** **** **** **** **** **** **** **** **** **** **** **** */
/* List Util */


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
        
        if( findPid( in->pid) != -1)
        {
            Message_buf outBuffer;
            UAVObject outObject;
            dumbUAVObject( &outObject );
            outBuffer.mtype = IPC_DataSend;
            dumbUAVObject(&outObject);
            
            strcpy((char*)outObject.data, "response");
            memcpy(outBuffer.data.buffer , &outObject , sizeof(UAVObject) );
            

            if ( IPC_send(&port, &outBuffer, sizeof(Message_buf)) < 0)
            {
                perror("send");
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
            const int8_t ret = IPC_selectRead( &port);
            if( ret == IPC_noerror )
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
            else if( ret == IPC_timeout )
            {
                outBuffer.mtype = IPC_DataSend;
                dumbUAVObject(&outObject);
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
        done = 0;
    }

    printf("Close Server \n");
    
    return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */



/*
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "../../libDaemon/Private/IPCMessage.h"







int main()
{
    
    initList();
    

    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    
    Message_buf sbuf;
    Message_buf  rbuf;
    size_t buf_length;

    key = IPC_KEY;

    if ((msqid = msgget(key, msgflg )) < 0)
    {
        perror("msgget");
        return -1;
    }
    else
        fprintf(stderr,"msgget: msgget succeeded: msqid = %d\n", msqid);
    

    
    sbuf.mtype = 1;
    
    buf_length = sizeof( Message_buf ) - sizeof(long);

    while (1)
    {
        sbuf.mtype = IPC_DataSend;


        if (msgrcv( msqid, &rbuf, buf_length , 0, IPC_NOWAIT ) > 0)
        {
            
            if( rbuf.mtype == IPC_ProcessRegistration)
            {
                

                addPid( rbuf.pid );

                printf("New registration from %i , reply count %i \n" , rbuf.pid, getNumClients() );

                addPid( rbuf.pid);
                printList();                
                sbuf.mtype = IPC_ProcessDidRegister;
                if (msgsnd(msqid, &sbuf, buf_length, 0) == 0)
                {
                    usleep( 2000);
                }
                

            }
            
            else if( rbuf.mtype == IPC_ProcessDeregistration)
            {

                removePid( rbuf.pid);

                printf("New DEregistration from %i count %i \n" , rbuf.pid, getNumClients() );
                printList();

            }
            else if( rbuf.mtype == IPC_DataRequest)
            {
                
                UAVObject obj;// =(UAVObject*) &rbuf.data.buffer ;
                initUAVObject( &obj );
                parseIPC(&rbuf, &obj);
                
                dumbUAVObject( (UAVObject*) &sbuf.data.buffer );
                
                strcpy( ((char*)((UAVObject*) &sbuf.data.buffer)->data), "Hello World");
                
                
                ((UAVObject*) &sbuf.data.buffer)->objectID = obj.objectID;
                
                if ( (findPid(rbuf.pid) != -1) && msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) == 0)
                {
                    //printf("Respond to data Request %u ... \n" , obj.objectID);
                }
            }
            
        }
        else
        {


        }


        

    }

}
*/