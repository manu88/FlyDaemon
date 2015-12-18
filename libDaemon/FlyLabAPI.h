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

typedef void (*event_uav)( const UAVObject *obj);
typedef void (*event_error)( int errorNum );
    
struct _FlyLabParameters
{
    event_uav function;
    event_error errorCallBack;
    
    void *userData;
};
    
typedef struct _FlyLabParameters FlyLabParameters;

//static event_uav function = NULL;

uint8_t initializeConnection( const FlyLabParameters *parameters );
void cleanup(void);
    

uint8_t disconnect( void );

uint8_t runFromThisThread( void );
uint8_t runFromNewThread(void);

uint8_t isConnected( void );

uint8_t sendObject( const UAVObject * obj);
    
uint8_t sendObjectRequest( uint32_t objectID);
    
    
#ifdef __cplusplus
} // closing brace for extern "C"
#endif


#endif /* FlyLabAPI_h */
