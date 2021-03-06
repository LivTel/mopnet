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
#define FAC FAC_MOP

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
    char  msg_rcv[1024]; // Receive message buffer
    char  msg_snd[1024]; // Send message buffer 
    char  msg_cpy[1024]; // Copy of command line options forwarded to slave appended with run number
    int   msg_len;
    int   run;           // Run number

//  Substitution arguments for re-parsing
    char *args[64];         

//  Log to screen by default and get process name
    log_fp = stdout;
    mop_proc = basename(argv[0]);

//  Parse command line options and init. data
    mop_log( mop_defs(), LOG_DBG, FAC, "mop_defs()"); 
    mop_log( mop_opts(argc, argv, CAM_ARGS, CAM_CHKS ), LOG_DBG, FAC, "mop_opts()"); 
    mop_log( mop_init(), LOG_DBG, FAC, "mop_init()"); 

//  Swap argument storage to local array for re-parsing
    argv = args; 

    if ( mop_master )              
    {
//      Initalisations 
        mop_log( msg_init ( ipmaster       ), LOG_DBG, FAC, "msg_init()" );   
        mop_log( whl_init ( whl_pos,TMO_WHL), LOG_DBG, FAC, "whl_init()" );
        mop_log( cam_init ( cam_num        ), LOG_DBG, FAC, "cam_init()" );
        mop_log( cam_open ( cam            ), LOG_DBG, FAC, "cam_open()" );
        mop_log( cam_conf ( cam, cam_exp   ), LOG_DBG, FAC, "cam_conf()" );
        mop_log( cam_alloc( cam            ), LOG_DBG, FAC, "cam_alloc()");
        mop_log( cam_cool ( cam, cam_temp, TMO_TOK, cam_quick ), LOG_DBG, FAC, "cam_cool()");

//      Forever loop
        for(;;)
        { 
//          Wait for RUN message 
            while( !mop_log( msg_recv( 0, msg_rcv, sizeof(msg_rcv), &msg_len, MSG_RUN, strlen(MSG_RUN) ), LOG_DBG, FAC, "msg_recv(%s)", msg_rcv ));

//          Copy message for forwarding to Slave 
            strncpy( msg_cpy, msg_rcv, sizeof(msg_cpy)-1 );  

//          Re-parse options, re-init data and re-configure camera
            argc=utl_msg2arg ( argv, msg_rcv, &msg_typ );   
            mop_log( mop_opts( argc, argv, CAM_ARGS, CAM_CHKS), LOG_DBG, FAC, "mop_opts()"); 
            mop_log( mop_init(                               ), LOG_DBG, FAC, "mop_init()"); 
            mop_log( whl_conf( whl_pos, TMO_WHL              ), LOG_DBG, FAC, "whl_conf()");
            mop_log( cam_conf( cam, cam_exp                  ), LOG_DBG, FAC, "cam_conf()");

//          Init. rotator to start position 
            mop_log( rot_init( rot_usb, ROT_BAUD, TMO_ROTATOR, ROT_TRG_HI ), LOG_DBG, FAC, "rot_init()"); 

//          Queue images, re-check temperature is still OK 
            mop_log( cam_queue ( cam ), LOG_DBG, FAC, "cam_queue()");
            mop_log( cam_cool( cam, cam_temp, TMO_TOK, cam_quick ), LOG_DBG, FAC, "cam_cool()");

//          Init. filename, get next available local run number 
            fts_run = FTS_INIT;
            mop_log( !fts_mkname( cam, fts_pfx, &fts_run ), LOG_DBG, FAC, "fts_mkname(INIT)");

//          KLUDGE: Append a suggested local rUn number onto message to slave as -U option
            sprintf( &msg_cpy[ strlen(msg_cpy) ], " -U%i", fts_run ); 

//          If using all cameras then wait for slave temperature stable OK 
            if ( !one_cam )
            {
//              Forward arguments to slave and await message indicating stable temperature
                mop_log( msg_send( TMO_MSG, msg_cpy, ipslave, MSG_ACK, strlen(MSG_ACK)), LOG_MSG, FAC,"msg_send(%s)", msg_cpy ); 
                mop_log( msg_recv( TMO_TOK, msg_rcv, sizeof(msg_rcv), &msg_len, MSG_TOK, strlen(MSG_TOK)), LOG_MSG, FAC,"msg_recv(%s)", MSG_TOK );

//              If slave supplied a run number extract it and compare
                if ( msg_len > strlen(MSG_TOK) )
                {
                    sscanf( msg_rcv, MSG_TOK" %d", &run );  
                    if ( run > fts_run ) 
                    {
                        mop_log( !fts_mkname( cam, fts_pfx, &run ), LOG_WRN, FAC, "Master RUN=%i low. Using Slave RUN=%i)", fts_run, run);
                        fts_run = run;
                    }
                }
            }

//          CAUTION: AT_Command can take > 0.5s (!) to complete so call any fn() using them before rotation
//          Reset camera clock and enable acquisition
            mop_log( cam_clk_rst( cam         ), LOG_DBG, FAC, "cam_clk_rst()"    );  
            mop_log( cam_acq_ena( cam, AT_TRUE), LOG_DBG, FAC, "cam_acq_ena(true)");  

//          If not single camera, signal slave that rotation is starting
            if ( !one_cam )
                mop_log(msg_send(TMO_ACK,MSG_ROT,ipslave,MSG_ACK,strlen(MSG_ACK)),LOG_MSG,FAC,"msg_send(%s)",MSG_ROT); 

//          Position rotator and start selected action 
            if ( !rot_sign &&  // 0 = Static 
                 !rot_stp    ) // 0 = Single position
            {
//              No rotation, single static position, software trigger (test mode) 
                mop_log( rot_goto( rot_zero, TMO_ROTATOR, &rot_zero ), LOG_INF, FAC, "Static position=%f", rot_zero );
                mop_log( cam_acq_stat( cam                          ), LOG_DBG, FAC, "cam_acq_stat(FIXED ANGLE)");
            }
            else if ( rot_sign ) // Rotating with hardware triggering (normal mode) 
            {
//              Enable hardware trigger, start rotation, circular acquisition 
                mop_log( rot_trg_ena( true ), LOG_DBG, FAC, "rot_trg_ena(true)" );
                mop_log( rot_move(rot_final), LOG_DBG, FAC, "rot_move(final)"   );
                mop_log( cam_acq_circ(cam  ), LOG_DBG, FAC, "cam_acq_circ()"    );
                mop_log( rot_trg_ena( false), LOG_DBG, FAC, "rot_trg_ena(false)"); 
            }
            else // Rotating with software triggering (alternate test mode) 
            {
                mop_log( cam_acq_stat( cam ), LOG_DBG, FAC, "cam_acq_stat(ROTATING)");
            }
        }
    }
    else // Running as slave
    {
//      Network, camera and image inits
        mop_log( msg_init ( ipslave      ), LOG_DBG, FAC, "msg_init()" );   
        mop_log( cam_init ( cam_num      ), LOG_DBG, FAC, "cam_init()" );
        mop_log( cam_open ( cam          ), LOG_DBG, FAC, "cam_open()" );
        mop_log( cam_conf ( cam, cam_exp ), LOG_DBG, FAC, "cam_conf()" );
        mop_log( cam_alloc( cam          ), LOG_DBG, FAC, "cam_alloc()");
        mop_log( cam_cool ( cam, cam_temp, TMO_TOK, cam_quick ), LOG_DBG, FAC, "cam_cool()");

//      Forever loop
        for(;;)
        { 
//          Wait for run message 
            while( !mop_log(msg_recv(0,msg_rcv,sizeof(msg_rcv),&msg_len,MSG_RUN,strlen(MSG_RUN)),LOG_DBG,FAC,"msg_recv(%s)",msg_rcv));

//          Extract run-time arguments, re-parse and re-init
            argc = utl_msg2arg( argv, msg_rcv, &msg_typ );   
            mop_log( mop_opts ( argc, argv, CAM_ARGS, CAM_CHKS), LOG_DBG, FAC, "mop_opts(Re-parse)"); 
            mop_log( mop_init (                               ), LOG_DBG, FAC, "mop_init(Re-init)" ); 
            mop_log( cam_conf ( cam, cam_exp                  ), LOG_DBG, FAC, "cam_conf(Re-conf)" );

//          Queue images, re-check temperature 
            mop_log( cam_queue ( cam ), LOG_DBG, FAC, "cam_queue()" );
            mop_log( cam_cool( cam, cam_temp, TMO_TOK, cam_quick ), LOG_DBG, FAC, "cam_cool()");

//          Init. filename for this run 
            run = FTS_INIT;           
            mop_log( !fts_mkname( cam, fts_pfx, &run ), LOG_DBG, FAC, "fts_mkname(INIT)");

//          Use Master RUN number f greater than Slave
            if ( fts_run > run )
                mop_log( !fts_mkname( cam, fts_pfx, &fts_run ), LOG_WRN, FAC, "Using Master RUN=%i", fts_run);
            else
                fts_run = run; // Else use Slave RUN        

//          Tell Master which RUN number is being used
            sprintf( msg_snd, MSG_TOK" %i", fts_run ); 

//          Tell master temperature is OK and wait for ACK
            mop_log( msg_send( TMO_MSG, msg_snd, ipmaster, MSG_ACK, strlen(MSG_ACK)), LOG_MSG, FAC,"msg_send(%s)",msg_snd); 

//          Reset camera clock and enable acquisition
            mop_log( cam_clk_rst( cam          ), LOG_DBG, FAC, "cam_clk_rst()" );  
            mop_log( cam_acq_ena( cam, AT_TRUE ), LOG_DBG, FAC, "cam_acq_ena(T)");  

//          Synchronise on rotation starting
            mop_log( msg_recv(TMO_ROT,msg_rcv,sizeof(msg_rcv),&msg_len,MSG_ROT,strlen(MSG_ROT)),LOG_MSG,FAC,"msg_recv(%s)",MSG_ROT); 

//          Acquire images	
            if ( rot_sign )
                mop_log( cam_acq_circ( cam ), LOG_DBG, FAC, "cam_acq_circ()");
            else
                mop_log( cam_acq_stat( cam ), LOG_DBG, FAC, "cam_acq_stat()");
        } 
    }
}
