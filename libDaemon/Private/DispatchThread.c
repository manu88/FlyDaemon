//
//  DispatchThread.c
//  DIspatcher
//
//  Created by Manuel Deneu on 04/12/2015.
//  Copyright © 2015 Manuel Deneu. All rights reserved.
//


#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "Dispatch.h"
#include "IPCMessage.h"

/*
 PRIVATE FILE
 */




int initDispatchThread( DispatchThread *dispatch )
{
    if( dispatch == NULL)
        return -1;
    
    pthread_mutex_init( &dispatch->mutex, NULL);
    
    dispatch->shouldQuit = 0;

    dispatch->r_key = -1;

    
    dispatch->error_ipc = 0;
    dispatch->error_thread = 0;
    
    return 1;
}

/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

int16_t connectToIPCWithKey( DispatchThread *dispatch , key_t key)
{
    if( dispatch->r_key != -1 )
        return 0; // already connected
    
    if (( dispatch->r_msqid = msgget( key, 0666)) < 0)
    {
        perror("msgget");
        dispatch->error_ipc = dispatch->r_msqid;
        return -1; // unable to connect
    }
    
    dispatch->r_key = key;
    
    return 1;
    
}

/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

uint8_t IPCIsConnected( DispatchThread *dispatch )
{
    return dispatch->r_key > 0;
}

/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

void *startMainLoop (void * p_data)
{
    
    dispatch_MainLoop( p_data );

    return NULL;
}

/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

void dispatch_MainLoop( void* dispatcher )
{



    const pid_t pid = getpid();

    printf(" My PID : %i \n" , pid );
    
    /**/
    GrandDispatcher *dispatch = (GrandDispatcher*) dispatcher;
    

    Message_buf  rbuf;
    Message_buf  sbuf;
    
    const size_t msgSize = sizeof( Message_buf ) - sizeof(long);

    
    /* Send registration request */
    
    sbuf.mtype = IPC_ProcessRegistration;
    sbuf.pid = pid;
    
    
    if( sendIPCMessage( &dispatch->_thread, &sbuf ) == 0)
    {
        printf("Error while sending IPC_ProcessRegistration message \n");

    }
    
    /* Wait for reply ... */
    
    uint8_t didReply = 0;
    uint32_t counter = 0;
    const uint32_t maxTime = 400000;
    while ( counter < maxTime )
    {
        if (msgrcv( dispatch->_thread.r_msqid, &rbuf, msgSize , IPC_ProcessDidRegister , IPC_NOWAIT ) > 0)
        {
            if ( rbuf.mtype == IPC_ProcessDidRegister)
            {
                didReply = 1;
                break;
            }
        }

        usleep( 10 );
        counter++;
        
    }
    
    if( didReply == 0)
    {
//        printf(" TimeOut : Server did not reply .. \n");
        GD_unlockDispatch( dispatch );
        dispatch->_callBack1( ConnectionError , dispatch->_callBackUserData1 );
        GD_lockDispatch( dispatch );

    }
    else // we're good!
    {
        
        dispatch->running = 1;
        
        GD_unlockDispatch( dispatch );
        dispatch->_callBack1( DidRegisterToDispatcher , dispatch->_callBackUserData1 );
        GD_lockDispatch( dispatch );
        
        while ( dispatch->_thread.shouldQuit == 0 )
        {
            if (msgrcv( dispatch->_thread.r_msqid, &rbuf, msgSize , 0 , IPC_NOWAIT ) > 0)
            {
                //printf("mtype %li \n", rbuf.mtype );
                
                if( dispatch->_callBack1)
                {
                    
                    if( rbuf.mtype == IPC_DataSend )
                    {
                        GD_unlockDispatch( dispatch );
                        dispatch->_callBack1( DidReceiveData , dispatch->_callBackUserData1 );
                        GD_lockDispatch( dispatch );
                    }
                    /*
                    else if ( rbuf.mtype == IPC_ProcessDidRegister)
                    {
                        GD_unlockDispatch( dispatch );
                        dispatch->_callBack1( DidRegisterToDispatcher , dispatch->_callBackUserData1 );
                        GD_lockDispatch( dispatch );
                    }
                     */
                    else
                    {
                        
                    }
                }
                

            }
            else // other tasks
            {
                GD_unlockDispatch( dispatch );
                if( dispatch->threadedLoop == 0)
                {
                    if( dispatch->_userTaskCallBack != NULL)
                        dispatch->_userTaskCallBack(dispatch->_userTaskData );
                }
                else
                    usleep(100);
                GD_lockDispatch( dispatch );
            }
            
        } // end of while
        

        sbuf.pid = pid;
        
        sbuf.mtype = IPC_ProcessDeregistration;
        if(sendIPCMessage( &dispatch->_thread, &sbuf) == 0)// msgsnd(dispatch->_thread.r_msqid, &sbuf, msgSize, 0) == 0)
        {
            
        }
    }
    
    dispatch->running = 0;

}

/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

uint8_t waitForThreadTerminaison( DispatchThread *dispatch )
{
    const int ret = pthread_join ( dispatch->thread_id, NULL);
    
    pthread_mutex_destroy( &dispatch->mutex);
    return ret == 0? 1 : 0;
    
}




int sendIPCMessage(DispatchThread *dispatch, const void *message )
{
    const size_t msgSize = sizeof( Message_buf ) - sizeof(long);
    
    const ssize_t ret =msgsnd(dispatch->r_msqid, message, msgSize, 0);
    if (ret == -1)
    {
        const int err = errno;
        printf("Erno msgsnd %i - %s\n " , err , strerror( err ));
    }
    
    return ret == 0? 1 : 0;
    
}
