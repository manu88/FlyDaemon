#include <stdio.h>
#include <stdlib.h>


#include <unistd.h>
#include "../../libDaemon/FlyLabAPI.h"



static void testCall( int reason, void* data)
{
    GrandDispatcher* dispatch = (GrandDispatcher*) data;
    
    static int count = 0;
    
    if( reason == ConnectionError )
    {
        printf("Connection error quit\n");
    }

    else if( reason == DidReceiveData )
    {
        printf("Haz data %i \n" , count);
    }
    else if( reason == DidRegisterToDispatcher )
    {
        printf("DidRegisterToDispatcher \n");
    }
    else
    {
        printf("Other Reason %i" , reason);
    }
    count++;


    if( count > 100)
    {
        GD_stop( dispatch);
    }

}

void userTask( void* data )
{
    static int counter = 0;
    
    GrandDispatcher *dispatch = ( GrandDispatcher*)data;
    
    printf("Send data request %i\n", counter);
    
    const char test[] = "hello";
    
    if(GD_sendMessage( dispatch ,(void*)test , 6))
        counter++;
}


int main (void)
{
    GrandDispatcher* dispatch = GD_init();

    GD_setCallBack1( dispatch, testCall, dispatch );
    

    GD_setUserTaskCallBack( dispatch , userTask, dispatch );
    
    GD_runFromLoop( dispatch );
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


    GD_release( dispatch );
    
    
    return EXIT_SUCCESS;
}
