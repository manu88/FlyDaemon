//
//  DispatchThread.c
//  DIspatcher
//
//  Created by Manuel Deneu on 04/12/2015.
//  Copyright © 2015 Manuel Deneu. All rights reserved.
//

#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>

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

//    dispatch->r_key = -1;

    
    dispatch->error_ipc = 0;
    dispatch->error_thread = 0;
    


    IPC_initialize( &dispatch->_port);
    IPC_createClient( &dispatch->_port);
    
    return 1;
}

void releaseDispatchThread(  DispatchThread *dispatch )
{
    if( dispatch == NULL)
        return;
    
    pthread_mutex_destroy( &dispatch->mutex );
    
    IPC_closeClient( &dispatch->_port );

}

/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */
/*
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

uint8_t IPCIsConnected( DispatchThread *dispatch )
{
    return dispatch->r_key > 0;
}
*/
/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

void *startMainLoop (void * p_data)
{
    
    dispatch_MainLoop( p_data );

    return NULL;
}

/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

void dispatch_MainLoop( void* dispatcher )
{
    GrandDispatcher *dispatch = (GrandDispatcher*) dispatcher;
    
    if( IPC_connectToServer( &dispatch->_thread._port ) != IPC_noerror)
        return;
    
    const pid_t pid = getpid();

    /**/

    dispatch->state = 1;
    
    Message_buf  rbuf;
    Message_buf  sbuf;
    
    /* Send registration request */
    
    sbuf.mtype = IPC_ProcessRegistration;
    sbuf.pid = pid;
    
    
    if( IPC_send(&dispatch->_thread._port, &sbuf, sizeof(Message_buf))<=0)
    {
        printf("Error send IPC_ProcessRegistration \n");
    }
    
    /* Wait for reply ... */
    
    uint8_t didReply = 0;
    uint32_t counter = 0;
    const uint32_t maxTime = 40;
    
    ssize_t t = 0;
    
    while ( counter < maxTime )
    {
        if( IPC_selectRead( &dispatch->_thread._port ) == IPC_noerror )
        {
            if ((t = IPC_receive( &dispatch->_thread._port, &rbuf, sizeof(Message_buf) ) > 0))
            {
                if ( rbuf.mtype == IPC_ProcessDidRegister)
                {
                    didReply = 1;
                    break;
                }
            }
        }

        counter++;
        
    }
    
    if( didReply == 0)
    {
        dispatch->_callBack1( ConnectionError ,NULL,  dispatch->_callBackUserData1 );
    }
    else // we're good!
    {
        

        dispatch->state = 2;
        
        dispatch->_callBack1( DidRegisterToDispatcher ,NULL, dispatch->_callBackUserData1 );
        
        /* Send an internal info request */
        
        sbuf.mtype = IPC_PrivateRequest;
        IPC_send( &dispatch->_thread._port , &sbuf, sizeof(Message_buf ));
        

        while ( dispatch->_thread.shouldQuit == 0 )
        {

            const int8_t ret = IPC_selectRead( &dispatch->_thread._port);
            if(  ret == IPC_noerror )
            {
                if ((t = IPC_receive(&dispatch->_thread._port, &rbuf, sizeof(Message_buf ))> 0))
                {

                    if( dispatch->_callBack1)
                    {

                        if( rbuf.mtype == IPC_DataSend )
                        {
                            dispatch->_callBack1( DidReceiveData ,&rbuf, dispatch->_callBackUserData1 );
                        }

                        else if(rbuf.mtype >= IPC_PrivateRequest )
                        {
                            dispatch->_callBack1( (int) rbuf.mtype ,&rbuf, dispatch->_callBackUserData1 );
                        }

                    }


                }
                else
                {
                    
                    printf("Read error, quit! %i \n" , dispatch->_thread._port.lastReceiveError);
                    GD_stop( dispatch );

                }
            }
            else if( ret == IPC_timeout)
            {
                if( dispatch->threadedLoop == 0)
                {
                    if( dispatch->_userTaskCallBack != NULL)
                        dispatch->_userTaskCallBack(dispatch->_userTaskData );
                }
            }
            
        } // end of while
        
        dispatch->_callBack1( WillTerminateConnection , NULL , dispatch->_callBackUserData1 );
    
        sbuf.pid = pid;
        
        sbuf.mtype = IPC_ProcessDeregistration;
        if( IPC_send(&dispatch->_thread._port, &sbuf, sizeof(Message_buf)) <= 0)
        {
        }
    }
    
    dispatch->state = 0;

}

/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

uint8_t waitForThreadTerminaison( DispatchThread *dispatch )
{
    const int ret = pthread_join ( dispatch->thread_id, NULL);
    
    pthread_mutex_destroy( &dispatch->mutex);
    return ret == 0? 1 : 0;
    
}



