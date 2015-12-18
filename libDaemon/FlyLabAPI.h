//
//  FlyLabAPI.h
//  Dispatcher
//
//  Created by Manuel Deneu on 17/12/2015.
//  Copyright © 2015 Manuel Deneu. All rights reserved.
//

#ifndef FlyLabAPI_h
#define FlyLabAPI_h
/*
 PUBLIC HEADER
 */



/*
 
 This is the main Header for the API
 
 */

#include "UAVTalk.h"

extern "C"
{
#include "Dispatch.h"
}
static GrandDispatcher *instance = NULL;


int initializeConnection( void );

int setCallback( event_cb_t function);

int disconnect( void );

int runFromThisThread( void );
int runFromNewThread(void);

int isConnected( void );

int sendObject( const UAV_Object * obj);


#endif /* FlyLabAPI_h */
