/**
* \file FlyLabAPI.h
* \brief main API header
* \author Manuel Deneu
* \version 0.1
* \date 17/12/2015
*
*  PUBLIC HEADER - This is the main Header for the API.
*  @example testClient.c
*
*/


#ifndef FlyLabAPI_h
#define FlyLabAPI_h

#ifdef __cplusplus
extern "C" {
#endif

#include "Commons.h"
#include "UAVTalk.h"
    

    
/**
 * A list of notifications types passed to notificationsCallBack
 * \enum PlateformType
 */
enum PlateformType
{
    Plateform_Unknown   = 0, /*!< Error : unknown error */
    Plateform_Drone     = 1,  /*!< A real flying system */
    Plateform_Simulator = 2   /*!< A simulated flying system */
};
    

    
/**
 *  Callback signature for receiving UAV Objects
 */
typedef void (*event_uav)( const UAVObject *obj , void* userData);
    
/**
 *  Callback signature for receiving state notifications
 */
typedef void (*event_error)( int errorNum , void* userData);
    
    
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/**
 *  This struct will hold the user parameters.
 *  \struct FlyLabParameters
 */
typedef struct
{
    event_uav parseObjectsCallBack;     //!< The user callback for receiving UAV Objects
    event_error notificationsCallBack;  //!< The user callback for receiving Notifications
    
    void *userData; //!< A pointer to anything you want to pass with user callback
} FlyLabParameters;
    

/** The maximum length of plateform & constructor names
 *  \def NAME_MAX_SIZE
 * \see RuntimeInformations
 */
#define NAME_MAX_SIZE 20
    
/**
 *  This struct will hold The runtime informations about the system.
 *  \see getRuntimeInformations informationsAvailable
 *  \struct RuntimeInformations
 */
typedef struct
{
    uint8_t plateform;        //!< Is this a real flying machine or a simulator \see PlateformType
    /* byte pad here */
    char name[NAME_MAX_SIZE];        //!< The name of the system
    char constructor[NAME_MAX_SIZE]; //!< The constructor of the system

    uint8_t versionMin; //!< version minor value
    uint8_t versionMaj; //!< version major value
    
    int8_t  hardwareStatus; //!< Status code sent by the hardware
    
} RuntimeInformations ;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    
/**
 * A list of notifications types passed to notificationsCallBack
 * \enum NotificationId
 */
enum NotificationId
{
    /* Must match DispatcherNotifications in Dispatch.h ! Do not reorder*/
    Connection_Error         = -1, /*!< A fatal error occured during connection. */
    Connection_OK            =  1, /*!< Connected. */
    Connection_WillEnd       =  2, /*!< Connection will end. */

    
    /* Limit*/
//    DidReceiveData          = 20
    
    InformationsAvailable    = 100
};
    
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

//! \brief Initialize a FlyLab communication
/*!
This is just initialization, you still need to start the communication at some point.
 \see runFromThisThread runFromNewThread
 
 \param[in] parameters an instance of FlyLabParameters containing callback parameters.
 
 \return 1 on sucess, 0 if failed.
 */
BOOLEAN_RETURN uint8_t initializeConnection( const FlyLabParameters *parameters );

    
//! \brief Clean a FlyLab communication
/*!
Will fail if initializeConnection wasn't called or failed.
 */
void cleanup(void);

//! \brief Get the return value from the dispatcher.
/*!
 Only relevant after the dispatcher has stopped!
 \return The return value from the dispatcher
 */
ALWAYS_INLINE int8_t getReturnValue(void);
    
//! \brief Get the current version
/*!
 \return a c-string containing the current version, e.g. "0.010"
 */
ALWAYS_INLINE const char* API_getVersion(void);

//! \brief Check if Daemon informations are available
/*!
 \see getRuntimeInformations
 
 \return 1 if true, 0 if not
 */
BOOLEAN_RETURN ALWAYS_INLINE uint8_t informationsAvailable(void);
    
//! \brief get runtime informations
/*!
 \see informationsAvailable RuntimeInformations
 \return a pointer to a RuntimeInformations readonly instance, can be NULL
 */
const RuntimeInformations* getRuntimeInformations( void );

//! \brief Ask for communication end
/*!
 This call is asynchronous, a stop call will be issued and the dispatcher will still run for some short time

 \return 1 on sucess, 0 if failed.
 */
BOOLEAN_RETURN ALWAYS_INLINE uint8_t disconnect( void );

//! \brief Run the dispatcher from main thread.
/*!
 This call will block until disconnect is issued
 \return 1 on sucess, 0 if failed.
 */
BOOLEAN_RETURN uint8_t runFromThisThread( void );
    
//! \brief Run the dispatcher from a new thread.
/*!
 This call will block and return after the thread initialization.
 
 \return 1 on sucess, 0 if failed.
 */
BOOLEAN_RETURN uint8_t runFromNewThread(void);

//! \brief Check for communication state
/*!
 
 In case of a threaded Dispatch, you can use this in your main Thread
 \code{.c}
 FlyLabParameters params;
 // set parameters ...

 initializeConnection( &params );
 
 if( runFromNewThread() == 1)
 {
    while ( isConnected() != 1) // Wait for connection
    {
        usleep( 1000 );
    }
    while ( isConnected() )
    {
        // do stuff
    }
    // thread ended here
 }
 \endcode
 
 \return 1 if connected, 0 if not.
 */
BOOLEAN_RETURN ALWAYS_INLINE uint8_t isConnected( void );

BOOLEAN_RETURN ALWAYS_INLINE int8_t sendObject(const UAVObject * obj);
    
//! \brief Send a UAVObject Request
/*!
 \param[in] objectID the requested object's ID.
 
 \return 1 on sucess, 0 if send failed.
 */
BOOLEAN_RETURN int8_t sendObjectRequest( uint32_t objectID);

    
//! \brief Send a UAVObject 'acknowledge response'
/*!
 \param[in] instanceID the requested object's instance ID.
 
 \return 1 on sucess, 0 if send failed.
 */
int8_t respondAcknowledge( uint16_t instanceID );
    
//! \brief Send a UAVObject 'negative acknowledge response'
/*!
 \param[in] instanceID the requested object's instance ID.
 
 \return 1 on sucess, 0 if send failed.
 */
int8_t respondNacknowledge( uint16_t instanceID );
    
//! \brief Try to lock the dispatcher thread
/*!
 \return 0 on sucess, otherwise an error code is returned. \see pthread_mutex_lock
 */
ALWAYS_INLINE int tryLockThread( void );
    
//! \brief Unlock the dispatcher thread
/*!
 \return 0 on sucess, otherwise an error code is returned. \see pthread_mutex_unlock
 */
ALWAYS_INLINE int unlockThread( void );
    
#ifdef __cplusplus
} // closing brace for extern "C"
#endif


#endif /* FlyLabAPI_h */
