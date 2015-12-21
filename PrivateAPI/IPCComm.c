//
//  IPCComm.c
//  Daemon
//
//  Created by Manuel Deneu on 19/12/2015.
//  Copyright © 2015 Manuel Deneu. All rights reserved.
//
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <errno.h>

#include "IPCComm.h"

const char SOCK_PATH[] = "/var/tmp/socket";

// will need other def for linux

#if defined(__APPLE__) && defined(__MACH__)
#define FLAG_NO_SIG_PIPE SO_NOSIGPIPE
#elif defined(__linux__)
#define FLAG_NO_SIG_PIPE MSG_NOSIGNAL
#endif

int8_t setCommonSocketOption( IPCCommunicationPort *port );





int8_t IPC_initialize( IPCCommunicationPort *port)
{
    
    port->_commSoc = -1;
    port->_serverSoc = -1;
    
    port->lastSendError = 0;
    port->lastReceiveError = 0;
    
    if ( ( port->_commSoc = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("_commSoc");
        return IPC_socket;
    }
    
    
    port->_timer = -1;
    port->_timeToWait = 5;
    
    
    return setCommonSocketOption( port );
}


int8_t IPC_createClient( IPCCommunicationPort *port)
{
    port->_remote.sun_family = AF_UNIX;
    
    strcpy( port->_remote.sun_path, SOCK_PATH);
    

    /*
    int flags = fcntl( port->_commSoc ,F_GETFL,0);
    fcntl( port->_commSoc , F_SETFL, flags | O_NONBLOCK);
    */
    return IPC_noerror;
}

int8_t IPC_connectToServer( IPCCommunicationPort *port)
{
    socklen_t len =(socklen_t) strlen(port->_remote.sun_path) + sizeof( port->_remote.sun_family );
    
    
    if (connect( port->_commSoc , (struct sockaddr *)&port->_remote, len) == -1)
    {
        perror("connect");
        port->connected = 0;
        return  IPC_connect;
    }
    
    port->connected = 1;
    return IPC_noerror;
}

int8_t IPC_createServer( IPCCommunicationPort *port)
{
    remove( "/var/tmp/socke" );
    
    if ( ( port->_serverSoc = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("_serverSoc");
        return IPC_socket;
    }
    
    socklen_t len = 0;
    
    port->_local.sun_family = AF_UNIX;
    strcpy( port->_local.sun_path, SOCK_PATH );
    
    

    unlink( port->_local.sun_path );
    
    len = (socklen_t) strlen( port->_local.sun_path) + sizeof(port->_local.sun_family);
 
    if (bind( port->_serverSoc , (struct sockaddr *)&port->_local, len) == -1)
    {
        perror("bind _serverSoc ");
        return IPC_bind;
    }
    
    if (listen( port->_serverSoc , 5) == -1)
    {
        perror("listen _serverSoc");
        return IPC_listen;
    }
    
    return IPC_noerror;
}


int IPC_closeClient( IPCCommunicationPort *port)
{
    return close( port->_commSoc );
}

int8_t IPC_closeServer( IPCCommunicationPort *port)
{
    int ret = close( port->_commSoc );
//    ret += close( port->_serverSoc );
    return ret == 0? 0  : IPC_close;
}

int8_t IPC_waitForClient(IPCCommunicationPort *port , struct timeval * timout)
{
    if( timout )
    {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET( port->_serverSoc , &rfds);

        const int ret = select( port->_serverSoc + 1, &rfds, NULL, NULL, timout);
        if( ret == 0)
            return IPC_timeout;
        else if( ret < 0 )
            return IPC_select;
    }
    socklen_t t = 0;
    
    t = sizeof(port->_remote);
    
    if (( port->_commSoc = accept( port->_serverSoc, (struct sockaddr *)&port->_remote, &t)) == -1)
    {
        perror("accept");
        port->connected = 0;
        return IPC_accept;
    }
    
    /*
    int flags = fcntl( port->_commSoc ,F_GETFL,0);
    fcntl( port->_commSoc , F_SETFL, flags | O_NONBLOCK);
    */

    setCommonSocketOption( port );
    

    port->connected = 1;
    
    return IPC_noerror;
}

int8_t setCommonSocketOption( IPCCommunicationPort *port)
{
#if defined(__APPLE__) && defined(__MACH__)
    int on = 1;
    if (setsockopt( port->_commSoc, SOL_SOCKET, FLAG_NO_SIG_PIPE, &on, sizeof(on)) == -1)
    {
        perror("setsockopt");
        return IPC_otherError;
    }
#endif
    /*
    int flags = fcntl( port->_commSoc ,F_GETFL,0);
    fcntl( port->_commSoc , F_SETFL, flags | O_NONBLOCK);
     */
    
    return IPC_noerror;
}

ssize_t IPC_send( IPCCommunicationPort *port, const void* buffer , size_t size)
{
    const ssize_t n = send(port->_commSoc, buffer, size, FLAG_NO_SIG_PIPE );

    port->lastSendError = errno;
    
    return n;
}

int8_t IPC_selectRead(IPCCommunicationPort *port )
{
    
    /* fd_set manages an array of sockets */
    fd_set          readSockets;
    struct timeval  tval;
    
    FD_ZERO(&readSockets);
    FD_SET(port->_commSoc, &readSockets);
    
    tval.tv_sec  = port->_timeToWait / 1000;
    tval.tv_usec = (port->_timeToWait % 1000) * 1000;
    
    const int ret = select(port->_commSoc + 1, &readSockets, NULL, NULL, &tval);
    
    if( ret == 0)
        return IPC_timeout;
    else if( ret < 0 )
        return IPC_select;
    

    return IPC_noerror;
    
}
ssize_t IPC_receive( IPCCommunicationPort *port , void * buffer, size_t size)
{
    const ssize_t n = recv( port->_commSoc, buffer, size, 0);
    
    port->lastReceiveError = errno;
    return n;
}

