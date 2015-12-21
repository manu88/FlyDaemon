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

#ifndef ALWAYS_INLINE
#define ALWAYS_INLINE __attribute__((always_inline))
#endif

#endif /* Commons_h */
