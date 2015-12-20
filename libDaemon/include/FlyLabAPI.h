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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    
typedef void (*event_uav)( const UAVObject *obj , void* userData);
typedef void (*event_error)( int errorNum , void* userData);
    
    
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

struct _FlyLabParameters
{
    event_uav parseObjectsCallBack;
    event_error notificationsCallBack;
    
    void *userData;
};
    
typedef struct _FlyLabParameters FlyLabParameters;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    
    
enum NotificationId
{
    /*
        Must match DispatcherNotifications in Dispatch.h !
     Do not reorder
     */
    Connection_Error         = -1,
    Connection_OK            = 1,
    Connection_WillEnd       = 2,
    /* Limit*/
//    DidReceiveData          = 20
};
    
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

uint8_t initializeConnection( const FlyLabParameters *parameters );
void cleanup(void);
    

uint8_t disconnect( void );

uint8_t runFromThisThread( void );
uint8_t runFromNewThread(void);

uint8_t isConnected( void );

int8_t sendObject(const UAVObject * obj);
int8_t sendObjectRequest( uint32_t objectID);
    
int8_t respondAcknowledge( uint16_t instanceID );
int8_t respondNacknowledge( uint16_t instanceID );
    
#ifdef __cplusplus
} // closing brace for extern "C"
#endif


#endif /* FlyLabAPI_h */
