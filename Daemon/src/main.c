#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


#include "../../libDaemon/Private/IPCMessage.h"



/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

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

/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

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
    
    
    /* Fist msg*/
    


    int i = 2;
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
                if ( (findPid(rbuf.pid) != -1) && msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) == 0)
                {
                    char buf[10];
                    strncpy(buf,  (char*)rbuf.data.buffer , 6);
                    

                    
                    printf("Respond to data Request'%s' ... \n" , buf);
                }
            }
            
        }
        else
        {
            /*
            if (msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) == 0)
            {
            
            }
             */

        }

        i++;
        if ( i > 5)
            i = 2;
        
        usleep( 100 );
    }

}