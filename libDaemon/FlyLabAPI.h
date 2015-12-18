//
//  FlyLabAPI.h
//  Dispatcher
//
//  Created by Manuel Deneu on 17/12/2015.
//  Copyright © 2015 Manuel Deneu. All rights reserved.
//

#ifndef FlyLabAPI_h
#define FlyLabAPI_h


/* DOC STYLE */

//! \brief Initialize empty UAVObject instance
/*!
 Everything will be set to zero in order to prevent garbage values
 
 \param obj must _not_ be NULL!
 
 \return void
 */



/*
 PUBLIC HEADER
    
 This is the main Header for the API
 */


#ifdef __cplusplus
extern "C" {
#endif

#include "UAVTalk.h"
#include "Dispatch.h"

static GrandDispatcher *instance = NULL;


int initializeConnection( void );

int setCallback( event_cb_t function);

int disconnect( void );

int runFromThisThread( void );
int runFromNewThread(void);

int isConnected( void );

int sendObject( const UAVObject * obj);
    
    
#ifdef __cplusplus
} // closing brace for extern "C"

#endif


#endif /* FlyLabAPI_h */
