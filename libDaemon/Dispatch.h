//
//  GrandDispatch.h
//  DIspatcher
//
//  Created by Manuel Deneu on 04/12/2015.
//  Copyright © 2015 Manuel Deneu. All rights reserved.
//

#ifndef GrandDispatch_h
#define GrandDispatch_h


#include "DispatchThread.h"

/*
 PUBLIC HEADER
 */



typedef void (*event_cb_t)(int reason, void *userdata);

typedef void (*event_userTask)( void *userdata);

typedef struct _GrandDispatcher
{
    DispatchThread _thread;
    
    uint8_t running; // flag : 0/1
    uint8_t threadedLoop; // 0 -> run from main thread, 1 -> run from dedicated thread;
   
    event_cb_t _callBack1;
    void*      _callBackUserData1;
    
    event_userTask _userTaskCallBack;
    void*          _userTaskData;
    
}GrandDispatcher;

enum DispatcherNotifications
{
    ConnectionError         = -1,
    
    DidRegisterToDispatcher = 1,
    DidReceiveData          = 2
};

GrandDispatcher* GD_init( void );
void GD_release( GrandDispatcher* dispatch);

uint8_t GD_runFromThread( GrandDispatcher *dispatch);
uint8_t GD_runFromLoop( GrandDispatcher *dispatch);

uint8_t GD_waitForCreation(GrandDispatcher *dispatch );

uint8_t GD_stop( GrandDispatcher* dispatch);

void GD_setCallBack1(GrandDispatcher* dispatch, event_cb_t function , void* userData);

/*
 In case you're using 'GD_runFromLoop', you can define an user function to be called.
 
 example:
    void userTask( void* data )
    {
        GrandDispatcher *dispatch = ( GrandDispatcher*)data;
        // .. do quick stuff
 }
 
 
 int main (void)
 {
    GrandDispatcher* dispatch = GD_init();
 
    GD_setUserTaskCallBack( dispatch , userTask, dispatch );
    GD_runFromLoop( dispatch ); // main thread is blocked here
 }
 
 Note : the userCallback will not be called if 'GD_runFromThread' is used instead.
 
 */
void GD_setUserTaskCallBack(GrandDispatcher* dispatch, event_userTask function , void* userData);

int GD_sendMessage(GrandDispatcher* dispatch , void* message , size_t size );

/* lock/unlock will do _nothing_ if dispatcher is called from the main thread*/
int GD_lockDispatch( GrandDispatcher* dispatch);
int GD_unlockDispatch( GrandDispatcher* dispatch);

#endif /* GrandDispatch_h */
