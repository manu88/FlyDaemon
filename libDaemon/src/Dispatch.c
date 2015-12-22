//
//  GrandDispatch.c
//  DIspatcher
//
//  Created by Manuel Deneu on 04/12/2015.
//  Copyright © 2015 Manuel Deneu. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include "Dispatch.h"

#include "IPCMessage.h"

/*
 PRIVATE FILE
 */

void *dispatch_MainLoop( void* dispatcher );

GrandDispatcher* GD_init()
{
    GrandDispatcher * ret =(GrandDispatcher*) malloc( sizeof(GrandDispatcher ));
    
    if (ret != NULL)
    {
        ret->state = 0;

        ret->_callBack1 = NULL;
        ret->_callBackUserData1 = NULL;
        
        ret->_userTaskData = NULL;
        ret->_userTaskCallBack = NULL;
        
        ret->_dispatcherUserData = NULL;
        
        initDispatchThread( &ret->_thread );
        ret->_thread.threadMainFunction = dispatch_MainLoop;
        
        //connectToIPCWithKey(&ret->_thread, IPC_KEY );

        return ret;
    }
    
    return NULL;
}

void GD_release( GrandDispatcher* dispatch)
{
    
    releaseDispatchThread( &dispatch->_thread );
    
    free( dispatch );
    
    dispatch = NULL;
}

void GD_setDispatchMethod( GrandDispatcher *dispatch , dispatcherFunction function , void* data)
{
    dispatch->_dispatchMainFunction = function;
    dispatch->_dispatcherUserData = data;
}

BOOLEAN_RETURN uint8_t GD_stop( GrandDispatcher* dispatch)
{
    if (dispatch->state == 0)
    {
        return 0;
    }
    
    sendQuitSignal( &dispatch->_thread );
    
    uint8_t ret = 0;
    
    if( dispatch->threadedLoop == 1)
         ret = waitForThreadTerminaison( &dispatch->_thread );
    
    dispatch->state = 0;

    return ret;
}

/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

BOOLEAN_RETURN uint8_t GD_runFromThread( GrandDispatcher *dispatch)
{
    if( dispatch->state != 0)
        return 0;
    
    const int r = startThread(&dispatch->_thread , dispatch);
    if( r == 0)
    {
        dispatch->threadedLoop = 1;
        return 1;
    }
    else
        return 0;
}

ERROR_RETURN int8_t GD_runFromLoop( GrandDispatcher *dispatch)
{
    if( dispatch->state != 0)
        return -1;
    
    dispatch->threadedLoop = 0;
    dispatch_MainLoop( dispatch );
    

    
    return dispatch->returnValue;
}

/* **** **** **** **** **** **** **** */

enum
{
    MAXCOUNT = 10000, // total waited time = MAXCOUNT * SLEEP_TIME
    SLEEP_TIME = 100 // uS
};

BOOLEAN_RETURN uint8_t GD_waitForCreation(GrandDispatcher *dispatch )
{
    uint16_t maxCount = 0;
    while( dispatch->state < 1)
    {
        usleep( SLEEP_TIME );
        if(maxCount++ > MAXCOUNT)
            break;
    }
    
    if( dispatch->state >= 1)
        return 1;
    
    // timeout here
    return 0;
    
}

/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

ALWAYS_INLINE void sendQuitSignal( DispatchThread *dispatch )
{
    if( dispatch->shouldQuit != 1)
        dispatch->shouldQuit = 1;
}

/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */


void GD_setCallBack1(GrandDispatcher* dispatch, event_cb_t function , void* userData)
{
    dispatch->_callBack1 = function;
    dispatch->_callBackUserData1 = userData;
    
}

/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

void GD_setUserTaskCallBack(GrandDispatcher* dispatch, event_userTask function , void* userData)
{
    dispatch->_userTaskCallBack = function;
    dispatch->_userTaskData = userData;
}

/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

ERROR_RETURN int8_t GD_sendMessage(GrandDispatcher* dispatch, void* message , size_t size )
{
    if( GD_tryLockDispatch(dispatch) == 0)
    {
        
        Message_buf msg;
    
        msg.mtype = IPC_DataRequest;
        msg.pid = getpid();

        memcpy(msg.data.buffer , message , size );

        const int8_t ret = IPC_send( &dispatch->_thread._port,&msg, size) > 0? 1 : 0;
        GD_unlockDispatch( dispatch );
        
        return ret;
    }
    
    return -2;
}

/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */

ERROR_RETURN ALWAYS_INLINE int GD_tryLockDispatch( GrandDispatcher* dispatch)
{
    return pthread_mutex_trylock( &dispatch->_thread.mutex );
}
ERROR_RETURN ALWAYS_INLINE int GD_lockDispatch( GrandDispatcher* dispatch)
{
    return pthread_mutex_lock( &dispatch->_thread.mutex );
}

ERROR_RETURN ALWAYS_INLINE int GD_unlockDispatch( GrandDispatcher* dispatch)
{
    return pthread_mutex_unlock( &dispatch->_thread.mutex );
}

/* **** **** **** **** **** **** **** **** **** **** **** **** **** **** */
/* Dispatch's entry Point                                                */

void* dispatch_MainLoop( void* dispatcher )
{
    GrandDispatcher *dispatch = (GrandDispatcher*) dispatcher;
 

    dispatch->state = 0;
    
    dispatch->returnValue = dispatch->_dispatchMainFunction( dispatch->_dispatcherUserData );
    
    dispatch->state = 0;
    
    return NULL;
}

