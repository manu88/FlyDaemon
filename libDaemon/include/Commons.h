/**
 * \file Commons.h
 * \brief Commons Definitions
 * \author Manuel Deneu
 * \version 0.1
 * \date 21/12/2015
 *
 *  PUBLIC HEADER - Commons definitions
 *
 */

#ifndef Commons_h
#define Commons_h

#ifndef ALWAYS_INLINE
#define ALWAYS_INLINE __attribute__((always_inline))
#endif

/** Use this macro on unused parameters to silence compiler warning Complains.
 *  \def UNUSED_PARAMETER
 */
#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif

/** This macro marks API functions that return true/false results as 0 or 1 (usually as uint8_t )
 *  \def BOOLEAN_RETURN
 */
#ifndef BOOLEAN_RETURN
#define BOOLEAN_RETURN
#endif

/** This macro marks API functions that return 0 on sucess or an error code on fail (usually as int8_t ) 
 *  \see ErrorNum
 *
 *  \def ERROR_RETURN
 */
#ifndef ERROR_RETURN
#define ERROR_RETURN
#endif

/**
 * Error code
 * \enum ErrorNum
 *
 * \see BOOLEAN_RETURN
 */
enum ErrorNum
{
    Error_No = 0, /*!< No errors. */
    
};


//! \brief debug mode checker
/*!
 \see isInReleaseMode
 
 \return 1 if the program is build whith DEBUG, 0 otherwise.
 */
BOOLEAN_RETURN static inline uint8_t isInDebugMode()
{
#ifdef DEBUG
    return 1;
#else
    return 0;
#endif
}

//! \brief release mode checker
/*!
 \see isInDebugMode
 
 \return 0 if the program is build whith DEBUG, 1 otherwise.
 */
BOOLEAN_RETURN static inline uint8_t isInReleaseMode()
{
    return !isInDebugMode();
}

/** Use this macro to assert your code.
 * Warning : when building for release, DEBUG_ASSERT will be turned into a no-op, so be carefull not to put any real
 * operations in it.
 * \code{.c} 
 *  // Wrong way
 *  DEBUG_ASSERT( runFromNewThread() == 1 );
 *  // on release build , the instruction 'runFromNewThread()' will simply diseapear!.
 *
 *  // Good here
 *  const uint8_t ret =runFromNewThread();
 *  DEBUG_ASSERT( ret == 1 );
 *
 * \endcode
 * \def DEBUG_ASSERT
 */
#ifdef DEBUG
    #include <assert.h>
    #define DEBUG_ASSERT(x) assert(x)

#else
    #define DEBUG_ASSERT(x)
#endif

#endif /* Commons_h */
