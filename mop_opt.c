/** @file   mop_opt.c
  *
  * @brief  MOPTOP process initialisation and command line parser   
  *
  * @author asp 
  *
  * @date   2019-10-19 
  */

#include "mopnet.h"
#define FAC FAC_OPT

/** @brief     Graceful exit 
  *
  * @param[in] code = exit code to be returned by process to shell 
  *
  * @return    code = exit code 
  */
void mop_exit( int code  )
{
    cam_close( &mop_cam ); // Tidy-up camera settings
    exit( code );
}


/** @brief     Process initialisation
  *
  * @return    true
  */
bool mop_defs( void )
{
    strncpy( fts_obj, FTS_OBJ, MAX_STR-1 );
    strncpy( fts_ra , FTS_RA , MAX_STR-1 );
    strncpy( fts_dec, FTS_DEC, MAX_STR-1 );
    strncpy( fts_dir, FTS_DIR, MAX_STR-1 );
    strncpy( log_pfx, LOG_PFX, MAX_STR-1 );

    return true;
}


/** @brief     Process initialisation
  *
  * @return    true
  */
bool mop_init( void )
{

//  Counter clockwise steps are negative
    if ( rot_sign == ROT_CCW )
       rot_stp = -fabs( rot_stp );

//  Calculate rotator final position depending on direction
    img_total = img_cycle * rot_revs; 
    if (      rot_sign == ROT_CW )
        rot_final = img_total * rot_stp - ROT_TOLERANCE; // Avoid trigger overrun by stopping short 
    else if ( rot_sign == ROT_CCW )
        rot_final = img_total * rot_stp + ROT_TOLERANCE; // Avoid trigger overrun by stopping short 

//  Approx. guess, more accurate later after reading camera data
//  cam_exp = fabs((rot_stp / rot_vel));
    if ((  rot_sign != ROT_STAT           )&&  // Not static
        ( !cam_auto                       )&&  // and not automatic exposure
        (  fabs(rot_stp/rot_vel) < cam_exp)  ) // and exposure will overrun
        mop_log( true, LOG_WRN, FAC,
                "Exposure %f will overrun trigger %f interval", cam_exp, rot_stp/rot_vel );

    return true;
}


/** @brief     -h Help. Prints run-time options and default values. 
  *
  */
void prn_opt( void )
{
    printf("Opt:                                [ Default       ]\n"           );
    if ( cam_auto )
        printf("  -e  Exposure time                 [ Automatic     ]\n"         );
    else
        printf("  -e  Exposure time                 [% 6.3f sec     ]\n",cam_exp );
    printf("  -x  eXosure type   <b,d,e,f,q,s>  [     %c         ]\n" , fts_pfx  );
    printf("  -b  Binning        <1,2,3,4,8>    [   %ls         ]\n"  , cam_bin  );
    printf("  -f  read Freq.     <100,270>      [   %ls     ]\n"      , cam_mhz  );
    printf("  -m  Mode amp. gain <12H,12L,16L>  [   %ls ]\n"          , cam_amp  );
    printf("  -p  Pixel encoding <12,12PACK,16> [   %ls      ]\n"     , cam_enc  );
    printf("  -n  Images per rev <8,16>         [% 6i         ]\n"    , img_cycle);
    printf("  -o  read Order   <BUSEQ, BUSIM,   [   %ls ]\n"          , cam_rd   );
    printf("      COSIM, OISIM, TDSEQ, TDSIM>\n"                                 );
    printf("  -r  rotator Revolutions           [% 6i         ]\n"    , rot_revs );
    printf("  -v  rotator Velocity <+ve,0,-ve>  [% 6.1f deg/sec ]\n"  , rot_vel  );
    printf("  -u  rotator USB device            [ %s  ]\n"            , rot_usb  );
    printf("  -t  target Temperature            [ <% 2.1f C       ]\n", cam_temp );
    printf("  -q  Quick start <0=false,1=true>  [ %5.5s         ]\n"  , btoa(cam_quick));
    printf("  -c  Camera <1=Master, 2=Slave>    [     %i         ]\n" , cam_num+1);
    printf("  -M  Master IP:port                [ %s ]\n"             , ipmaster );
    printf("  -S  Slave  IP:port                [ %s ]\n"             , ipslave  );
    printf("  -W  Write destination             [    %s/         ]\n" , fts_dir  );
    printf("  -w  filter Wheel position <1-5>   [    %i          ]\n" , whl_pos  );
    printf("     <1=%s, 2=%s, 3=%s, 4=%s, 5=%s>\n",
               whl_colour[1], whl_colour[2], whl_colour[3], whl_colour[4], whl_colour[5]);
    printf("  -N  filter wheel device Name      [ %s  ]\n"            , whl_dev  );
    puts("Obs:");
    printf("  -O  Obj. name                     [ %-13.13s ]\n"       , fts_obj  );
    printf("  -R  Obj. RA  <hh:mm:ss.ss>        [ %-11.11s   ]\n"     , fts_ra   );
    printf("  -D  Obj. DEC <dd.mm.ss.ss>        [ %-12.12s  ]\n"      , fts_dec  );
    printf("  -F  Focus position                [% 8.3f       ]\n"    , tel_foc  );
    printf("  -C  CAS angle                     [% 8.3f       ]\n"    , tel_cas  );
    printf("  -A  Altitude angle                [% 8.3f       ]\n"    , tel_alt  );
    printf("  -Z  aZimuth angle                 [% 8.3f       ]\n"    , tel_azm  );
    puts("Dbg:");
    printf("  -d  Debug  +ve=Level, -ve=Module  [     %i         ]\n" 
           "     < %i=CRIT %i=SYS  %i=ERR  %i=WRN  %i=IMG  %i=INF  %i=MSG  %i=DBG>  %i=NONE\n"
           "      -%i=MOP -%i=LOG -%i=UTL -%i=OPT -%i=CAM -%i=ROT -%i=FTS -%i=MSG> -%i=WHL >\n",
                log_level,
                LOG_CRIT, LOG_SYS, LOG_ERR, LOG_WRN, LOG_IMG, LOG_INF, LOG_MSG, LOG_DBG, LOG_NONE,
                FAC_MOP , FAC_LOG, FAC_UTL, FAC_OPT, FAC_CAM, FAC_ROT, FAC_FTS, FAC_MSG, FAC_WHL );
    printf("  -i  Andor camera ID <1, 2>        [     %i         ]\n" , cam_idx+1 );
    printf("  -s  Force single master camera    [ %5.5s         ]\n"  , btoa(one_cam));
    printf("  -a  static fixed Angle            [  % 2.1f deg     ]\n", rot_zero );
    printf("  -l  Log file                      [ %8.8s      ]\n"     , log_file ? log_file:"<none>");
    printf("  -L  Log prefix                    [ %8.8s      ]\n"     , log_pfx  );
    printf("  -k  Kill process\n");
    printf("  -U  suggest rUn starting number (may be overriden if too low) \n");
    printf("  -E  Enumerate options for <feature>\n");
}


/** @brief     Process any command line options 
  *
  *            This is a long function to trap every optioni.
  *            Options marked RUNTIME can only be set at process start, not by a network message 
  *
  * @param[in] argc   = argument count 
  * @param[in] argv   = array of pointers to argument strings
  * @param[in] valid  = supported options  
  * @param[in] check  = check options
  *
  * @return    true | false = Success | Failure
  */
bool mop_opts(int argc, char *argv[], char *valid, char *check )
{
    int c;
    AT_WC Feature[255];
    char *ptr;
    char  cmd[1024];
    char  dir[1024];

    int    i; // Test integer
    double f; // Test real

    struct stat st = {0};

    opterr = 0; // Suppress errors
    optind = 1; // Set index to first arg

    while (( c = getopt( argc, argv, valid )) != -1 )
    {
        switch(c)
        {
            case 'r': // Set total number of revolutions  
                i = atoi(optarg); 
                if ( i < 1 || i > MAX_REVS ) 
                    return mop_log( false, LOG_ERR, FAC, "Rotations %s out-of-range, must be >0 and <%i", optarg, MAX_REVS);
                rot_revs = i; 
                break;
            case 'd': // Set debug level
                log_level = atoi(optarg);
                break;
            case 'e': // Set exposure time
                if ( optarg[0] == 'a' || optarg[0] == 'A' ) // Check if automatic
                {
                    cam_auto = true;
                    break;
                }
                f = atof(optarg);
                if ( f < 0.00001 || f > 30.0 ) 
                    return mop_log( false, LOG_ERR, FAC, "Exposure %s out-of-range. Use >= 0.00001 or <= 30.0", optarg); 
                cam_exp  = f;
                cam_auto = false;
		break;
            case 'q': // Don't wait for temp. to stabilise
                cam_quick = atoi(optarg) ? true : false;
		break;
            case 'E': // Display a detector enumerated feature
                mop_log( true, LOG_WRN, FAC, "DEBUG ONLY: -%c must be last option.",c); 
		mbstowcs( Feature, optarg, MAX_STR-1 );
		mop_init();
		cam_init(  cam_num );
		cam_open( &mop_cam );
		cam_conf( &mop_cam, cam_exp );
		cam_feature( &mop_cam, Feature );
                mop_exit( EXIT_SUCCESS );
		break;
	    case 'n': // Set number of steps in a rotation
                switch ( atoi(optarg) )
                {  
                    case 8:
                        rot_stp   = ROT_STP8;
                        img_cycle = 8;
                        break;
                    case 16:
                        rot_stp   = ROT_STP16;
                        img_cycle = 16;
                        break;
                    default:
                        return mop_log( false, LOG_ERR, FAC, "%s images per rev. unsupported. Use 8 or 16", optarg); 
                        break;
                }
                break;
            case 'u': // RUNTIME ONLY: Set the rotator USB device
                rot_usb = optarg;
                break;
            case 'U': // Suggest run starting number 
                fts_run = FTS_INIT;
                mop_log( !fts_mkname( &mop_cam, fts_pfx, &fts_run ), LOG_DBG, FAC, "fts_mkname(INIT)");
                if ( fts_run > atoi(optarg) ) 
                {
                    mop_log( false, LOG_WRN, FAC, "Run=%i too low, using run=%i ", atoi(optarg), fts_run ); 
                }
                else
                {
                    fts_run = atoi(optarg);
//                  mop_log( !fts_mkname( &mop_cam, fts_pfx, &fts_run ), LOG_DBG, FAC, "fts_mkname(FORCE=%i)", fts_run);
                }
                break;
            case 'v': // Set rotation velocity 
                f = atof(optarg);
                if ( fabs(f) > ROT_VEL_MAX )
                    return mop_log( false, LOG_ERR, FAC, "Rotator velocity %s out-of-range.", optarg ); 

                rot_vel = f;
                if      ( rot_vel  > 0.0 )   // Clockwise
                {
                    rot_sign = ROT_CW;       // Mark as clockwise
                    cam_trg  = CAM_TRG_EDGE; // Use hardware triggering
                }
                else if ( rot_vel  < 0.0 )   // Counter-clockwise
                {
                    rot_vel  = -rot_vel;     // Force velocity positive
                    rot_sign = ROT_CCW;      // Mark as counter clockwise
                    cam_trg  = CAM_TRG_EDGE; // Use hardware triggering
                }
                else if ( rot_vel == 0.0 )   // Static
                {
                    rot_sign = ROT_STAT;     // Mark as static
                    cam_trg  = CAM_TRG_SW;   // Use software triggering
                }
                break;
            case 't': // Set target temperature
                cam_temp = atof(optarg);
                break;
            case 'w': // Set filter wheel position 
                i = atoi(optarg);
                if ( i < 1 || i > 5 )
                    return mop_log( false, LOG_ERR, FAC, "Filter wheel position -w%i unsupported. Use 1 to 5"); 
                else
                    whl_pos = i;
                break;
            case 'N': // RUNTIME ONLY: Set the filter wheel USB device
                whl_dev = optarg;
                break;
            case 'c': // Camera number  
                i = atoi(optarg) - 1;
                if ( i < 0 || i > CAM_COUNT )
                    return mop_log( false, LOG_ERR, FAC, "Camera -c%s unsupported. Use 1 or %i", optarg, CAM_COUNT); 
                cam_num = i; 
                if ( cam_num == 0 )
                    mop_master = true;
                break;
            case 'f': // Select readout rate
                switch ( atoi(optarg) )
                {
                    case 100:
                        cam_mhz = CAM_MHZ_100; 
                        break;
                    case 270:
                        cam_mhz = CAM_MHZ_270; 
                        break;
                    default:
                        return mop_log( false, LOG_ERR, FAC, "Unsupported read rate=%s. Use 100 or 270", optarg); 
                        break; 
                }
                break;
            case 'o': // Select scan order 
                strtoupper( optarg );
                if      ( !strcmp( optarg, "BUSEQ" ))
                    cam_rd = CAM_RD_BUSEQ;
                else if ( !strcmp( optarg, "BUSIM" ))
                    cam_rd = CAM_RD_BUSIM;
                else if ( !strcmp( optarg, "COSIM" ))
                    cam_rd = CAM_RD_COSIM;
                else if ( !strcmp( optarg, "OISIM" ))
                    cam_rd = CAM_RD_OISIM;
                else if ( !strcmp( optarg, "TDSEQ" ))
                    cam_rd = CAM_RD_TDSEQ;
                else if ( !strcmp( optarg, "TDSIM" ))
                    cam_rd = CAM_RD_TDSIM;
                else 
                    return mop_log( false, LOG_ERR, FAC,
                                    "Unsupported read-out mode %s. Use BUSEQ, BUSIM, COSIM, OISIM, TDSEQ or TDSIM", optarg); 
                break;
            case 'm': // Select amplifier gain mode 
                strtoupper( optarg );
                if      ( !strcmp( optarg, "12H" ))
                    cam_amp = CAM_AMP_12H;
                else if ( !strcmp( optarg, "12L" ))
                    cam_amp = CAM_AMP_12L;
                else if ( !strcmp( optarg, "16L" ))
                    cam_amp = CAM_AMP_16L;
                else 
                    return mop_log( false, LOG_ERR, FAC, "Unsupported gain mode %s. Use 12H, 12L or 16L", optarg); 
                break;
            case 'a': // Fixed angle   
                f = atof(optarg);
                if ( f < -360.0  || f > 360.0 )
                    return mop_log( false, LOG_ERR, FAC, "Unsupported angle %s. Must be between -360 to +360.0 deg", optarg); 
                rot_zero = f;
                rot_sign = ROT_STAT;
                rot_vel  = ROT_VEL;
                rot_stp  = 0.0;
                cam_trg  = CAM_TRG_SW;
                mop_log( true, LOG_WRN, FAC, "DEBUG ONLY: -%c overrides other options, must be last", c); 
                break;
            case 'p': // Select data size
                strtoupper( optarg );
                if      ( !strcmp( optarg, "12"    ))
                    cam_enc = CAM_ENC_12;
                else if ( !strcmp( optarg, "12PACK"))
                    cam_enc = CAM_ENC_12PACK;
                else if ( !strcmp( optarg, "16"    ))
                    cam_enc = CAM_ENC_16;
                else 
                    return mop_log( false, LOG_ERR, FAC, "Invalid pixel encoding %s. Use 12, 12PACK or 16", optarg); 
                break; 
            case 'b': // Set binning
                switch ( img_bin=atoi(optarg) )
                {
                    case 1:
                        cam_bin = CAM_BIN_1;
                        img_bin = fts_ccdxbin = fts_ccdybin = 1;
                        break;
                    case 2:
                        cam_bin = CAM_BIN_2;
                        img_bin = fts_ccdxbin = fts_ccdybin = 2;
                        break;
                    case 3:
                        cam_bin = CAM_BIN_3;
                        img_bin = fts_ccdxbin = fts_ccdybin = 3;
                        break;
                    case 4:
                        cam_bin = CAM_BIN_4;
                        img_bin = fts_ccdxbin = fts_ccdybin = 4;
                        break;
                    case 8:
                        cam_bin = CAM_BIN_8;
                        img_bin = fts_ccdxbin = fts_ccdybin = 8;
                        break;
                    default: 
                        return mop_log( false, LOG_ERR, FAC, "Invalid binning %s. Use 1, 2, 3, 4 or 8", optarg); 
                        break;
                }               
                break;
            case 's': // DEBUG ONLY: Force single camera as master
                mop_master = true;                
                one_cam    = true;
                break;
            case 'x': // Set image type
                switch ( fts_pfx = optarg[0] )
                {
                    case FTS_PFX_BIAS:
                       fts_typ = FTS_TYP_BIAS;
                       break;
                    case FTS_PFX_DARK:
                       fts_typ = FTS_TYP_DARK;
                       break;
                    case FTS_PFX_EXP:
                       fts_typ = FTS_TYP_EXP;
                       break;
                    case FTS_PFX_FLAT:
                       fts_typ = FTS_TYP_FLAT;
                       break;
                    case FTS_PFX_ACQ:
                       fts_typ = FTS_TYP_ACQ;
                       break;
                    case FTS_PFX_STD:
                       fts_typ = FTS_TYP_STD;
                       break;
                    default: 
                       return mop_log( false, LOG_ERR, FAC, "Invalid image code %c. Use b,d,e,f,q, or s", optarg[0]); 
                       break; 
                }
                break;
            case 'W': // Set a destination write folder  
                ptr = optarg + strlen(optarg) - 1; // Point to last char
                if ( *ptr == '/' )                 // If it is a directory terminator char ...
                    *ptr = '\0';                   // ... erase it as we add our own later

//              Expand any home directory character
                if ( ptr = strchr( optarg, '~' ) ) 
                    sprintf( dir, "%s%s", getenv("HOME"), ++ptr ); 
                else
                    sprintf( dir, "%s", optarg );

//              Build a mkdir command
                sprintf( cmd, "mkdir -p %s", dir );

//              Create directory path and check for success
                if ( ( system( cmd )       == -1 )||  // system() didn't work
                     ( stat(dir, &st)      == -1 )||  // File doesn't exist
                     ( S_ISDIR(st.st_mode) ==  0 )  ) // Path is not a directory   
                {
                    mop_log( false, LOG_ERR, FAC, "Problem with destination=%s. Using default=%s/", dir, FTS_DIR ); 
                    strncpy( fts_dir, FTS_DIR, MAX_STR-1 );
                } 
                else
                {
                    mop_log( true, LOG_INF, FAC, "File destination=%s/", dir ); 
                    strncpy( fts_dir, dir, MAX_STR-1 );
                }
                break;
            case 'M': // RUNTIME ONLY:  Master IP address 
                if ( utl_chk_ip( optarg ) )
                     mop_log( true, LOG_INF, FAC, "MasterIP = %s", ipmaster=optarg); 
                 else
                     mop_log( false, LOG_WRN, FAC, "Invalid MasterIP %s", optarg); 
                break; 
            case 'S': // RUNTIME ONLY: Slave IP address
                if ( utl_chk_ip( optarg ) )
                     mop_log( true,  LOG_INF, FAC, "SlaveIP = %s", ipslave=optarg); 
                else
                     mop_log( false, LOG_WRN, FAC, "Invalid SlaveIP %s", optarg); 
                break; 
            case 'O': // Set object name 
                strncpy( fts_obj, optarg, MAX_STR-1 );
                break; 
            case 'R': // Set object RA 
                strncpy( fts_ra,  optarg, MAX_STR-1 );
                break; 
            case 'D': // Set object DEC 
                strncpy( fts_dec, optarg, MAX_STR-1 );
                break; 
            case 'F': // Set telescope focus position 
                tel_foc = atof(optarg);
                break; 
            case 'C': // Set telescope CAS angle 
                tel_cas = atof(optarg);
                break; 
            case 'A': // Set telescope Altitude angle 
                tel_alt = atof(optarg);
                break; 
            case 'Z': // Set telescope aZimuth angle 
                tel_azm = atof(optarg);
                break; 
            case 'L': // DEBUG ONLY: Logging prefix 
                strncpy( log_pfx, optarg, MAX_STR-1 );
                break; 
            case 'i': // Andor camera index 
                i= atoi(optarg);
                if ( i < 1 || i > 2 )
                     return mop_log( false, LOG_WRN, FAC, "Camera index %s out-of-range. Use 1 or 2", optarg); 
                cam_idx = --i;
                break; 
            case 'l': // RUNTIME ONLY: Log file 
                log_fp = fopen( optarg, "a" ); 
                if  ( !log_fp ) // ERROR: Restore default 
                {
                    log_fp = stdout;
                    mop_log( false, LOG_SYS, FAC, "Log file fopen(%s) %s", optarg, strerror(errno)); 
                }
                break; 
            case 'h': // Help! 
                printf("Usage: %-10.10s <-OPTION>         [ Version=%5.5s ]\n", argv[0], MOP_VERSION);
                prn_opt();
                puts("  -h  Help");
                puts("");
                mop_exit( EXIT_SUCCESS );
                break;
            case 'k': // Kill process   
                mop_kill = true; 
                if ( !strcmp( mop_proc, MOP_PROC )) // Only if mopcam
                    mop_exit( EXIT_SUCCESS );
                break;
            case '?': // Catch argument errors
                if ( strchr( check, optopt ) )
                    mop_log( false, LOG_WRN, FAC, "Option -%c missing argument", optopt); 
                else if ( isprint(optopt) )
                    mop_log( false, LOG_WRN, FAC, "Option -%c unsupported", optopt); 
                else
                    mop_log( false, LOG_WRN, FAC, "Option character -0x%02x unsupported", optopt); 
                break;
            default: // Something really bad so give up
                mop_exit( mop_log( EXIT_FAILURE, LOG_CRIT, FAC, "Invalid run options")); 
                break;
         }
    }

    return true;
}
