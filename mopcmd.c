/** @file mopcmd.c
  *
  * @brief MOPTOP command wrapper process. Passes run-time options to master server process   
  *
  * @author asp
  *
  * @date 2019-10-02
  */

#define MAIN
#include "mopnet.h"
#define FAC FAC_CMD

/** @brief     Main 
  *
  * @param[in] argc = argument count
  * @param[in] argv = argument variables
  *
  * @return    EXIT_SUCCESS 
  */
int main( int argc, char *argv[] )
{
    char  msg_buf[1024]; // Message buffer
    int   msg_len;
    int   total;

    log_fp = stdout; // Output to screen

    cam_num = -1;  // Command process is not a real camera

//  Parse command line arguments
    mop_log ( mop_opts(argc, argv, CMD_ARGS, CMD_CHKS ), LOG_DBG, FAC, "mop_opts()");
    msg_init( IPCOMMAND );

//  If -k kill option then send to both servers
    if ( mop_kill )
    {
        mop_log( msg_send( 1, utl_arg2msg( argc, argv, MSG_RUN), ipmaster, MSG_ACK, strlen(MSG_ACK) ), LOG_INF, FAC, "Kill Master");
        mop_log( msg_send( 1, utl_arg2msg( argc, argv, MSG_RUN), ipslave , MSG_ACK, strlen(MSG_ACK) ), LOG_INF, FAC, "Kill Slave" );
    }
    else 
    {
        total = rot_revs * img_cycle * 2; 

//      Send to Master process to be forwarded to Slave
        if (!mop_log( msg_send( 1, utl_arg2msg( argc, argv, MSG_RUN ), ipmaster, MSG_ACK, strlen(MSG_ACK) ), LOG_INF, FAC, "msg_send()"))
            return EXIT_FAILURE;        
        else
            printf( "Waiting for 2 x %i x %i = %i images ...\n", rot_revs, img_cycle, total );
 
//      Wait for expected number of images to be acquired
        for( int i = total; i--; )
        {
             if (!mop_log( msg_recv( 20, msg_buf, sizeof(msg_buf)-1, &msg_len, NULL, 0 ), LOG_DBG, FAC, "msg_recv()"))
                 return EXIT_FAILURE;
             if ( msg_len > 0 ) 
                 puts( msg_buf );
        }
    }

    return EXIT_SUCCESS;
}
