#include <stdio.h>
#include <stdlib.h>


#include <unistd.h>
#include "../../libDaemon/FlyLabAPI.h"



/*
void userTask( void* data )
{
    static int counter = 0;
    
    GrandDispatcher *dispatch = ( GrandDispatcher*)data;
    
    printf("Send data request %i\n", counter);
    
    const char test[] = "hello";
    
    if(GD_sendMessage( dispatch ,(void*)test , 6))
        counter++;
}

*/
static void uavObjectReceived( const UAVObject *obj )
{
    printf("UAV object received \n");
}
int main (void)
{
    FlyLabParameters params;
    params.function = uavObjectReceived;
    
    initializeConnection( &params );
    

    
    
//    runFromThisThread();
    runFromNewThread();
    
    
    while ( isConnected() )
    {
        usleep(10000);
        sendObjectRequest( 1);
    }
    /*
    GrandDispatcher* dispatch = GD_init();

    GD_setCallBack1( dispatch, testCall, dispatch );
    

    GD_setUserTaskCallBack( dispatch , userTask, dispatch );
    
    GD_runFromLoop( dispatch );
     */
/*

    GD_runFromThread( dispatch );
    
    if(GD_waitForCreation( dispatch ) == 0)
        printf("Error !\n");

    while( dispatch->running == 1)
    {
        userTask( dispatch );
        
        usleep(10000);
        
    }
*/

    printf("thread ended\n");

    cleanup();
//    GD_release( dispatch );
    
    
    return EXIT_SUCCESS;
}
