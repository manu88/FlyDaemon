/**
 * \file IPCComm.h
 * \brief Internal socket communication
 * \author Manuel Deneu
 * \version 0.1
 * \date 19/12/2015
 *
 *  PRIVATE HEADER
 */

#ifndef IPCComm_h
#define IPCComm_h

#include <stdint.h>
#include <sys/un.h>
#include <sys/time.h>

/**
 * A list of errors codes returned by every IPC_* functions
 *
 * This will give you a hint about the error, you should check erro, lastSendError and lastReceiveError for details
 *
 * \enum IPC_errors
 */
enum IPC_errors
{
    IPC_noerror = 0,
    IPC_socket  = -1,
    IPC_bind    = -2,
    IPC_listen  = -3,
    IPC_accept  = -4,
    IPC_connect = -5,
    IPC_close   = -6,
    IPC_select  = -7,
    IPC_timeout = -20,
    IPC_lock    = -21,
    IPC_refused = -22,
    
    IPC_brokenConnection = -30,
    
    IPC_otherError = -40
};

typedef struct
{
    struct sockaddr_un _remote; // both Clients & server
    int _commSoc;               // both Clients & server communication
    
    /* Server ONLY */
    struct sockaddr_un _local; // server ONLY
    int _serverSoc; // server ONLY listen to connections
    
    /* Timer */
    
    int _timer;
    long _timeToWait; // millis
    
    /* Stats */
    uint8_t connected; // 0/1
    
    int lastSendError;
    int lastReceiveError;
} IPCCommunicationPort;



/* Common init for Client & Server */
int8_t IPC_initialize( IPCCommunicationPort *port);
ssize_t IPC_send( IPCCommunicationPort *port, const void* buffer , size_t size);
ssize_t IPC_receive( IPCCommunicationPort *port , void * buffer, size_t size);

int8_t IPC_selectRead(IPCCommunicationPort *port );
int8_t IPC_selectWrite(IPCCommunicationPort *port );
/* Server part */
int8_t IPC_createServer( IPCCommunicationPort *port);
int8_t IPC_closeServer( IPCCommunicationPort *port);

//! \brief Wait for a client connection
/*!
 Will block until a client is connected, or timeout expires.
 
 \param port the IPCCommunicationPort data structure
 \param timout the time to wait before returning. If Null this method will have no timeout!
 
 \return IPC_errors : IPC_noerror on sucess , IPC_timeout if timer expires, IPC_accept if connection fails
 */
int8_t IPC_waitForClient(IPCCommunicationPort *port , struct timeval * timout);

/* Client part */

int8_t IPC_createClient( IPCCommunicationPort *port);
int IPC_closeClient( IPCCommunicationPort *port);
int8_t IPC_connectToServer( IPCCommunicationPort *port);
#endif /* IPCComm_h */
