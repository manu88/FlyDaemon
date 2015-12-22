/**
 * \file Dispatch.h
 * \brief Internal dispatcher
 * \author Manuel Deneu
 * \version 0.1
 * \date 04/12/2015
 *
 *  PRIVATE HEADER
 */


#ifndef GrandDispatch_h
#define GrandDispatch_h

#include "../include/Commons.h"

#include "DispatchThread.h"

/**
 *  Callback signature for dispatch notifications
 */
typedef void (*event_cb_t)(int reason, const void* msg, void *userdata);

/**
 *  Callback signature for user task
 */
typedef void (*event_userTask)( void *userdata);

typedef int8_t (*dispatcherFunction)(void *userdata);

/**
 *  An instance of GrandDispatcher
 *  \struct GrandDispatcher
 */
typedef struct
{
    DispatchThread _thread; //!< The internal thread
    
    uint8_t state;          //!< flag : 0 : not ready 1 : starting 2 : running;
    uint8_t threadedLoop;   //!< 0 -> run from main thread, 1 -> run from dedicated thread;
   
    event_cb_t _callBack1;          //!< a function called when receiving events \see GD_setCallBack1
    void*      _callBackUserData1;  //!< a pointer passed with the function \see GD_setUserTaskCallBack
    
    event_userTask _userTaskCallBack; //!< In case the GrandDispatcher in ran on the main loop, the user can define a method to be called as often as possible
    void*          _userTaskData; //!< a pointer passed with the user function
    
    dispatcherFunction  _dispatchMainFunction;
    void               *_dispatcherUserData;
    
    int8_t returnValue;
    
}GrandDispatcher;


/**
 * A list of notifications types to describe incoming events
 * \enum DispatcherNotifications
 */
enum DispatcherNotifications
{
    /*
     notifs < DidReceiveData are notifications , notifs >= DidReceiveData are Value dispatch
     */
    ConnectionError         = -1, //!< An error occured during connection, fatal!
    DidRegisterToDispatcher = 1,  //!< Current pid did register
    WillTerminateConnection = 2,  //!< Connection will be stopped, by the user or by the dispatcher

    /* Limit*/
    DidReceiveData          = 20, //!< Data was received
    
    PrivateInformationsUpdated = 100 //!< Internal informations updated
};


//! \brief Initialize a GrandDispatcher instance
/*!
 This is just initialization, you still need to start the dispatcher at some point.
 

 \return A pointer to the instance. You own this instance! \see GD_release
 */
GrandDispatcher* GD_init( void );


//! \brief Release a GrandDispatcher instance
/*!
 \param[in,out] dispatch A pointer to the instance you want to release. It will be NULLed.
 */
void GD_release( GrandDispatcher* dispatch);


void GD_setDispatchMethod( GrandDispatcher *dispatch , dispatcherFunction function , void* data);

//! \brief Start the dispatcher on a dedicated thread
/*!
 \param[in,out] dispatch A pointer to the instance you want to run
 \return 1 on sucess, 0 on fail
 */
BOOLEAN_RETURN uint8_t GD_runFromThread( GrandDispatcher *dispatch);

//! \brief Start the dispatcher on the current thread
/*!
 \param[in,out] dispatch A pointer to the instance you want to run
 \return -1 or Dispatch loop return value
 */
ERROR_RETURN int8_t GD_runFromLoop( GrandDispatcher *dispatch);

//! \brief Wait until the dispatch loop is fully running
/*!
 \param[in,out] dispatch A pointer to the instance you want to run
 \return 1 on sucess, 0 on fail
 */
BOOLEAN_RETURN uint8_t GD_waitForCreation(GrandDispatcher *dispatch );


//! \brief Send an asynchronous signal to stop the dispatcher
/*!
 \param[in,out] dispatch A pointer to the instance you want to run
 \return 1 on sucess, 0 on fail
 */
BOOLEAN_RETURN uint8_t GD_stop( GrandDispatcher* dispatch);

void GD_setCallBack1(GrandDispatcher* dispatch, event_cb_t function , void* userData);

//! \brief Set the user task callback
/*!
  In case you're using 'GD_runFromLoop', you can define an user function to be called regularly.
 
 \code{.c}
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
 \endcode
 
  Note : the userCallback will not be called if 'GD_runFromThread' is used instead.
 
 \param[in,out] dispatch A pointer to the instance you want to run
 \param[in] function the function to be called
 \param[in] userData the user pointer to be passed with the call
 */
void GD_setUserTaskCallBack(GrandDispatcher* dispatch, event_userTask function , void* userData);

ERROR_RETURN int8_t GD_sendMessage(GrandDispatcher* dispatch , void* message , size_t size );

/* lock/unlock will do _nothing_ if dispatcher is called from the main thread*/

//! \brief Try to lock GrandDispatch's mutex
/*!
 \see pthread_mutex_trylock man page for error codes
 \param[in,out] dispatch A pointer to the instance you want to run
 \return 0 on sucess, an error code on fail
 */
ERROR_RETURN ALWAYS_INLINE int GD_tryLockDispatch( GrandDispatcher* dispatch);

//! \brief Lock GrandDispatch's mutex
/*!
 \see pthread_mutex_lock man page for error codes
 \param[in,out] dispatch A pointer to the instance you want to run
 \return 0 on sucess, an error code on fail
 */
ERROR_RETURN ALWAYS_INLINE int GD_lockDispatch( GrandDispatcher* dispatch);

//! \brief Unlock GrandDispatch's mutex
/*!
 \see pthread_mutex_unlock man page for error codes
 \param[in,out] dispatch A pointer to the instance you want to run
 \return 0 on sucess, an error code on fail
 */
ERROR_RETURN ALWAYS_INLINE int GD_unlockDispatch( GrandDispatcher* dispatch);

#endif /* GrandDispatch_h */
