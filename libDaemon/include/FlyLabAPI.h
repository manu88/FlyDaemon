/**
* \file FlyLabAPI.h
* \brief main API header
* \author Manuel Deneu
* \version 0.1
* \date 17/12/2015
*
*  PUBLIC HEADER - This is the main Header for the API.
*
*/


#ifndef FlyLabAPI_h
#define FlyLabAPI_h


/* DOC STYLE */

//! \brief Initialize empty UAVObject instance
/*!
 Everything will be set to zero in order to prevent garbage values
 
 \param obj must _not_ be NULL!
 
 \return void
 */




#ifdef __cplusplus
extern "C" {
#endif

#include "UAVTalk.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    
/**
 *  Callback signature for receiving UAV Objects
 *
 *
 */
typedef void (*event_uav)( const UAVObject *obj , void* userData);
    
/**
 *  Callback signature for receiving state notifications
 *
 */
typedef void (*event_error)( int errorNum , void* userData);
    
    
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/**
 *  This struct will hold The user parameters.
 */
struct _FlyLabParameters
{
    event_uav parseObjectsCallBack;     //!< The user callback for receiving UAV Objects
    event_error notificationsCallBack;  //!< The user callback for receiving Notifications
    
    void *userData; //!< A pointer to anything you want to pass with user callback
};
    
typedef struct _FlyLabParameters FlyLabParameters;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    
/**
 * A list of notifications types passed to notificationsCallBack
 * \enum NotificationId
 */
enum NotificationId
{
    /* Must match DispatcherNotifications in Dispatch.h ! Do not reorder*/
    
    Connection_Error         = -1,
    Connection_OK            =  1,
    Connection_WillEnd       =  2,
    
    /* Limit*/
    
//    DidReceiveData          = 20
};
    
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

//! \brief Initialize a FlyLab communication
/*!
This is just initialization, you still need to start the communication at some point.
 \see runFromThisThread runFromNewThread
 
 \param[in] parameters an instance of FlyLabParameters containing callback parameters.
 
 \return 1 on sucess, 0 if failed.
 */

uint8_t initializeConnection( const FlyLabParameters *parameters );

    
//! \brief Clean a FlyLab communication
/*!
Will fail if initializeConnection wasn't called or failed.
 */
void cleanup(void);
    

//! \brief Ask for communication end
/*!
 This call is asynchronous, a stop call will be issued and the dispatcher will still run for some short time

 \return 1 on sucess, 0 if failed.
 */
uint8_t disconnect( void );

//! \brief Run the dispatcher from main thread.
/*!
 This call will block until disconnect is issued
 \return 1 on sucess, 0 if failed.
 */
uint8_t runFromThisThread( void );
    
//! \brief Run the dispatcher from a new thread.
/*!
 This call will block and return after the thread initialization.
 
 \return 1 on sucess, 0 if failed.
 */
uint8_t runFromNewThread(void);

//! \brief Check for communication state
/*!
 
 \return 1 if connected, 0 if not.
 */
uint8_t isConnected( void );

int8_t sendObject(const UAVObject * obj);
    
//! \brief Send a UAVObject Request
/*!
 \param[in] objectID the requested object's ID.
 
 \return 1 on sucess, 0 if send failed.
 */
int8_t sendObjectRequest( uint32_t objectID);
    
int8_t respondAcknowledge( uint16_t instanceID );
int8_t respondNacknowledge( uint16_t instanceID );
    
#ifdef __cplusplus
} // closing brace for extern "C"
#endif


#endif /* FlyLabAPI_h */