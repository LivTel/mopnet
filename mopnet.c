/** @file mopnet.c
  *
  * @brief MOPTOP main process, network synchronisation version
  *
  * @author asp
  *
  * @date 2019-10-04
  *
  * @version 0.25
  */

#define MAIN
#include "mopnet.h"

/** @brief      main() entry point
  *
  * @param[in]  argc   = argument count
  * @param[in] *argv[] = argument variables
  *
  * @return     none - Exit handled by mop_exit() 
  */
int main( int argc, char *argv[] )
{
    mop_cam_t *cam = &mop_cam; // Pointer to camera structure    

//  Network messaging
    char *msg_typ;
    char  msg_buf[1024]; // Message buffer
    char  msg_cpy[1024]; // For forwarding to slave
    int   msg_len;

//  Substitution arguments for re-parsing
    char *args[64];         

//  Log to screen by default and get process name
    log_fp = stdout;
    mop_proc = basename(argv[0]);

//  Parse command line options and init. data
    mop_log( mop_defs(), LOG_DBG, FAC_MOP, "mop_defs()"); 
    mop_log( mop_opts(argc, argv, CAM_ARGS, CAM_CHKS ), LOG_DBG, FAC_MOP, "mop_opts()"); 
    mop_log( mop_init(), LOG_DBG, FAC_MOP, "mop_init()"); 

//  Swap argument storage to local array for re-parsing
    argv = args; 

    if ( mop_master )              
    {
//      Inialisations 
        mop_log( msg_init ( ipmaster     ), LOG_DBG, FAC_MOP, "msg_init()" );   
        mop_log( cam_init ( cam_num      ), LOG_DBG, FAC_MOP, "cam_init()" );
        mop_log( cam_open ( cam          ), LOG_DBG, FAC_MOP, "cam_open()" );
        mop_log( cam_conf ( cam, cam_exp ), LOG_DBG, FAC_MOP, "cam_conf()" );
        mop_log( cam_alloc( cam          ), LOG_DBG, FAC_MOP, "cam_alloc()");
        mop_log( cam_cool ( cam, cam_temp, TMO_TOK, cam_quick ), LOG_DBG, FAC_MOP, "cam_cool()");

//      Forever loop
        for(;;)
        { 
//          Wait for RUN message 
            while( !mop_log( msg_recv( 0, msg_buf, sizeof(msg_buf), &msg_len, MSG_RUN ), LOG_DBG, FAC_MOP, "msg_recv(%s)", msg_buf ));

//          Copy message for forwarding to Slave 
            strncpy( msg_cpy, msg_buf, sizeof(msg_cpy)-1 );  

//          Re-parse options, re-init data and re-configure camera
            argc=utl_msg2arg ( argv, msg_buf, &msg_typ );   
            mop_log( mop_opts( argc, argv, CAM_ARGS, CAM_CHKS), LOG_DBG, FAC_MOP, "mop_opts()"); 
            mop_log( mop_init(                               ), LOG_DBG, FAC_MOP, "mop_init()"); 
            mop_log( cam_conf( cam, cam_exp                  ), LOG_DBG, FAC_MOP, "cam_conf()");

//          Init. rotator to start position 
            mop_log( rot_init( rot_usb, ROT_BAUD, TMO_ROTATOR, ROT_TRG_HI ), LOG_DBG, FAC_MOP, "rot_init()"); 
//          Queue images, re-check temperature is still OK 
            mop_log( cam_queue ( cam ), LOG_DBG, FAC_MOP, "cam_queue()");
            mop_log( cam_cool( cam, cam_temp, TMO_TOK, cam_quick ), LOG_DBG, FAC_MOP, "cam_cool()");

//          If using all cameras then wait for slave temperature stable OK 
            if ( !one_cam )
            {
//              Forward arguments to slave and wait for it to be stable 
                mop_log( msg_send( TMO_MSG, msg_cpy, ipslave, MSG_ACK ), LOG_MSG, FAC_MOP, "msg_send(%s)", msg_cpy ); 
                mop_log( msg_recv( TMO_TOK, msg_buf, sizeof(msg_buf), &msg_len, MSG_TOK ), LOG_MSG, FAC_MOP, "msg_recv(%s)", MSG_TOK);
            }

//          CAUTION: AT_Command can take > 0.5s (!) to complete so call any fn() using them before rotation
//          Reset camera clock and enable acquisition
            mop_log( cam_clk_rst( cam         ), LOG_DBG, FAC_MOP, "cam_clk_rst()"    );  
            mop_log( cam_acq_ena( cam, AT_TRUE), LOG_DBG, FAC_MOP, "cam_acq_ena(true)");  

//          If not single camera, signal slave that rotation is starting
            if ( !one_cam )
                mop_log( msg_send( TMO_ACK, MSG_ROT, ipslave, MSG_ACK ), LOG_MSG, FAC_MOP, "msg_send(%s)", MSG_ROT ); 

//          Init. filename for this run
            mop_log( !fts_mkname( cam, fts_pfx, true ), LOG_DBG, FAC_MOP, "fts_mkname(true)");

//          Position rotator and start selected action 
            if ( !rot_sign &&  // 0 = Static 
                 !rot_stp    ) // 0 = Single position
            {
//              No rotation, single static position, software trigger (test mode) 
                mop_log( rot_goto( rot_zero, TMO_ROTATOR, &rot_zero ), LOG_INF, FAC_MOP, "Static position=%f", rot_zero );
                mop_log( cam_acq_stat( cam                          ), LOG_DBG, FAC_MOP, "cam_acq_stat(FIXED ANGLE)");
            }
            else if ( rot_sign ) // Rotating with hardware triggering (normal mode) 
            {
//              Enable hardware trigger, start rotation, circular acquisition 
                mop_log( rot_trg_ena( true ), LOG_DBG, FAC_MOP, "rot_trg_ena(true)" );
                mop_log( rot_move(rot_final), LOG_DBG, FAC_MOP, "rot_move(final)"   );
                mop_log( cam_acq_circ(cam  ), LOG_DBG, FAC_MOP, "cam_acq_circ()"    );
                mop_log( rot_trg_ena( false), LOG_DBG, FAC_MOP, "rot_trg_ena(false)"); 
            }
            else // Rotating with software triggering (alternate test mode) 
            {
                mop_log( cam_acq_stat( cam ), LOG_DBG, FAC_MOP, "cam_acq_stat(ROTATING)");
            }
        }
    }
    else 
    {
//      Network, camera and image inits
        mop_log( msg_init ( ipslave      ), LOG_DBG, FAC_MOP, "msg_init()" );   
        mop_log( cam_init ( cam_num      ), LOG_DBG, FAC_MOP, "cam_init()" );
        mop_log( cam_open ( cam          ), LOG_DBG, FAC_MOP, "cam_open()" );
        mop_log( cam_conf ( cam, cam_exp ), LOG_DBG, FAC_MOP, "cam_conf()" );
        mop_log( cam_alloc( cam          ), LOG_DBG, FAC_MOP, "cam_alloc()");
        mop_log( cam_cool ( cam, cam_temp, TMO_TOK, cam_quick ), LOG_DBG, FAC_MOP, "cam_cool()");

//      Forever loop
        for(;;)
        { 
//          Wait for run message 
            while( !mop_log( msg_recv( 0, msg_buf, sizeof(msg_buf), &msg_len, MSG_RUN ), LOG_DBG, FAC_MOP, "msg_recv(%s)", msg_buf ));

//          Extract run-time arguments, re-parse and re-init
            argc = utl_msg2arg( argv, msg_buf, &msg_typ );   
            mop_log( mop_opts ( argc, argv, CAM_ARGS, CAM_CHKS), LOG_DBG, FAC_MOP, "mop_opts(Re-parse)"); 
            mop_log( mop_init (                               ), LOG_DBG, FAC_MOP, "mop_init(Re-init)" ); 
            mop_log( cam_conf ( cam, cam_exp                  ), LOG_DBG, FAC_MOP, "cam_conf(Re-conf)" );

//          Queue images, re-check temperature 
            mop_log( cam_queue ( cam ), LOG_DBG, FAC_MOP, "cam_queue()" );
            mop_log( cam_cool( cam, cam_temp, TMO_TOK, cam_quick ), LOG_DBG, FAC_MOP, "cam_cool()");

//          Tell master temperature is OK 
            mop_log( msg_send( TMO_MSG, MSG_TOK, ipmaster, MSG_ACK ), LOG_MSG, FAC_MOP, "msg_send(%s)", MSG_TOK); 

//          Reset camera clock and enable acquisition
            mop_log( cam_clk_rst( cam          ), LOG_DBG, FAC_MOP, "cam_clk_rst()" );  
            mop_log( cam_acq_ena( cam, AT_TRUE ), LOG_DBG, FAC_MOP, "cam_acq_ena(T)");  

//          Synchronise on rotation starting
            mop_log( msg_recv( TMO_ROT, msg_buf, sizeof(msg_buf), &msg_len, MSG_ROT ), LOG_MSG, FAC_MOP, "msg_recv(%s)", MSG_ROT); 

//          Init. filename for this run 
            mop_log( !fts_mkname( cam, fts_pfx, true ), LOG_DBG, FAC_MOP, "fts_mkname(T)");

//          Acquire images	
            if ( rot_sign )
                mop_log( cam_acq_circ( cam ), LOG_DBG, FAC_MOP, "cam_acq_circ()");
            else
                mop_log( cam_acq_stat( cam ), LOG_DBG, FAC_MOP, "cam_acq_stat()");
        } 
    }
}
