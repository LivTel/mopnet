/** @file   mopnet.h
  *
  * @brief  MOPTOP general header included by all modules
  *
  * @author asp 
  *
  * @date   2019-05-01 
  */

// Needed for extended dirent() functionality
#define _GNU_SOURCE

// Standard C headers 
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <semaphore.h>
#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>
#include <wchar.h> 

// System headers
#include <sys/dir.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

// Network headers
#include <arpa/inet.h>
#include <netinet/in.h>

// Non-standard third party headers
#include <PI_GCS2_DLL.h>  //!< Physik Instrumente rotator header
#include <atcore.h>	  //!< Andor camera access functions
#include <atutility.h>	  //!< Andor conversion utils (modified to work with C) 
#include <fitsio.h>	  //!< Starlink FITS header 

#define MOP_VERSION   "0.25"
#define MOP_DESCRIP   "Circular buffer. AT Retry. Network enabled. Remote kill." 
#define MOP_PROC      "mopnet" //!< Process name

#define MAX_STR       256 //!<  Maximum string length

// Argument options, OPTS and checks, CHKS 
// CAM = Camera run-time options, MSG = Message options, CMD = Command options       
#define OPTS_CAM      "u:l:c:E:i:h?msN"
#define OPTS_MSG      "e:x:b:f:p:n:o:r:v:t:q:S:M:W:O:R:D:F:C:A:Z:d:a:L:k"

#define CHKS_CAM      "ulcEihmsN"
#define CHKS_MSG      "exbfpnorvtqSMWORDFCAZdaLk"

#define CAM_ARGS      OPTS_MSG OPTS_CAM 
#define CAM_CHKS      CHKS_MSG CHKS_CAM
#define CMD_ARGS      OPTS_MSG   
#define CMD_CHKS      CHKS_MSG   

// Network addresses
//#define IPCOMMAND "192.168.1.28:12000" //!< <IP:port> Command process (ARI testing)
//#define IPMASTER  "192.168.1.28:12001" //!< <IP:port> Master  process (ARI testing)
//#define IPSLAVE   "192.168.1.29:12002" //!< <IP:port> Slave   process (ARI testing)  

#define IPCOMMAND "150.204.241.232:12000" //!< <IP:port> Command process (ARI testing)
#define IPMASTER  "150.204.241.232:12001" //!< <IP:port> Master  process (ARI testing)
#define IPSLAVE   "150.204.240.225:12002" //!< <IP:port> Slave   process (ARI testing) 
//#define IPSLAVE   "150.204.241.232:12002" //!< <IP:port> Slave   process (ARI testing)  

// Inter-process message types 
#define MSG_ACK   "ACK" //!< Acknowledge
#define MSG_NAK   "NAK" //!< Negative acknowledge
#define MSG_RDY   "RDY" //!< Process ready
#define MSG_RUN   "RUN" //!< Run messages 
#define MSG_TOK   "TOK" //!< Process sync, temperature OK
#define MSG_ROT   "ROT" //!< Process sync, rotation started
#define MSG_TRG   "TRG" //!< Process sync, SW trigger 
#define MSG_REJ   "REJ" //!< Request rejected 
#define MSG_IMG   "CAM" //!< Image written 

// Andor error ranges
#define AT_ERR_MIN 0
#define AT_ERR_MAX 39 
#define UT_ERR_MIN 1000
#define UT_ERR_MAX 1006 

// When Andor commands go wrong, retry
#define AT_TRY   3  //!< AT_ command max. retries
#define AT_DLY   1  //!< AT_ command retry delay  

// Log levels
#define LOG_NONE 0  //!< Suppress all log errors
#define LOG_CRIT 1  //!< Log Critical errors, process will exit 
#define LOG_SYS  2  //!< Log System errors, operation failed
#define LOG_ERR  3  //!< Log Programme errors, operation failed 
#define LOG_WRN  4  //!< Log Warnings, will continue 
#define LOG_IMG  5  //!< Log Image acquisition   
#define LOG_INF  6  //!< Log Informational  
#define LOG_MSG  7  //!< Log Message events 
#define LOG_DBG  8  //!< Debug logging, verbose  
#define LOG_CMD  9  //!< Log Command process events 

// Facilities (softwre module)
#define FAC_NUL  0  //!< Unused 
#define FAC_MOP  1  //!< Facility
#define FAC_LOG  2  //!< Logging
#define FAC_UTL  3  //!< Utilties
#define FAC_OPT  4  //!< Option parsing
#define FAC_CAM  5  //!< Camera operations
#define FAC_ROT  6  //!< Rotator operations
#define FAC_FTS  7  //!< FITS files
#define FAC_MSG  8  //!< Messaging
#define FAC_CMD  9     

// ANSI text colour 30=Black, 31=red, 32=green, 33=yellow, 34=blue, 35=magenta, 36=cyan, 37=white  
#define COL_RED     "\x1b[31m"  
#define COL_GREEN   "\x1b[32m"  
#define COL_YELLOW  "\x1b[33m"  
#define COL_BLUE    "\x1b[34m"  
#define COL_MAGENTA "\x1b[35m"    
#define COL_CYAN    "\x1b[36m"   
#define COL_WHITE   "\x1b[37m"    
#define COL_RESET   "\x1b[0m"    
#define COL_NULL    NULL    

// Space fillng for multi-informational lines
#define LOG_BLANK "\n\t\t\t\t  "
#define LOG_PFX   ""

//Timeouts
#define TMO_ROTATOR     30.0            //!< [s]  Timeout Rotator positioning 
#define TMO_TOK         60.0            //!< [s]  Timpout Sync. on temperature OK 
#define TMO_ROT         30.0            //!< [s]  Timeout Sync. on rotation start
#define TMO_ACK	        5               //!< [s]  Timeout Sync. ACK reply               
#define TMO_MSG         30              //!< [s]  Timeout Sync. on message
#define TMO_XFR         30000           //!< [ms] Timeout Image transfer  

// PI prefix = ROT_
#define ROT_BAUD        460800          //!< Baud rate 
#define ROT_USB         "/dev/ttyUSB0"  //!< Default USB device
#define ROT_STP8        45.0            //!< [deg] Step between triggers,  8 position  
#define ROT_STP16       22.5            //!< [deg] Step between triggers, 16 position
#define ROT_STP32       11.25           //!< [deg] Step between triggers, 32 postion 
#define ROT_VEL         45.0            //!< [deg/s] Default rotator velocity deg/sec 
#define ROT_REVS        3               //!< Default revolutions
#define ROT_ZERO        0.0             //!< [deg] Default starting position
#define ROT_TOLERANCE   0.004           //!< [deg] Rotator tolerance 
#define ROT_LIM_MAX     36000.0         //!< Limit: Maximum travel distance      
#define ROT_VEL_MAX     360.0           //!< Limit: Maximum velocity 

#define ROT_REBOOT      "RBT"           //!< Reboot rotator 
#define ROT_CLR_ERR     "STP"           //!< Stop all axis motion 
#define ROT_ALL_STOP    "ERR?"          //!< Clear error(s) 
#define ROT_INI_SVO     "SVO 1 1"       //!< Enable servo
#define ROT_INI_FRF     "FRF 1"         //!< Enable relative motion 
#define ROT_INI_VEL     "VEL 1 360"     //!< Default velocity at init. [deg/s] 
#define ROT_INI_ANGLE    22.0           //!< [deg] Initial angle (unsigned) 
#define ROT_INI_POS     "MOV 1 %f"      //!< Move to initial angle       
#define ROT_MOV         "MOV 1 %f"      //!< Move to position       

#define ROT_TRG_VEL     "VEL 1 %f"      //!< Angular velocity while triggering 
#define ROT_TRG_STP     "CTO 1 1 %f"    //!< Value: Trigger every 22.5 degrees of distance travel       
#define ROT_TRG_PIN_5   "CTO 1 2 1"     //!< Pin:   Trigger via output 1 (physical pin 5) on axis 1       
#define ROT_TRG_DST     "CTO 1 3 0"     //!< Mode:  Use distance travel for trigger events    
#define ROT_TRG_POSSTP  "CTO 1 3 7"     //!< Mode:  Use start position and distance travel for trigger events    
#define ROT_TRG_HI      "CTO 1 7 1"     //!< Trigger polarity high
#define ROT_TRG_LO      "CTO 1 7 0"     //!< Trigger polarity low
#define ROT_TRG_POSBEG  "CTO 1 8 0"     //!< Value: Trigger start when position=0.0 deg       
#define ROT_TRG_POSEND  "CTO 1 9 %f"    //!< Value: Trigger ends at this travel limit      
#define ROT_TRG_POSINIT "CTO 1 10 0"    //!< Value: Trigger enabled at position=0.0 deg       
#define ROT_TRG_LEN     "CTO 1 11 50"   //!< Trigger pulse width in 33.3ns increments
#define ROT_TRG_ENA     "TRO 1 1"       //!< Enable trigger       
#define ROT_TRG_DIS     "TRO 1 0"       //!< Disable trigger       

#define ROT_CW           1              //!< Clockwise rotation
#define ROT_CCW         -1              //!< Counter-clockwise rotation
#define ROT_STAT         0              //!< Static 

// FITS file definitions
#define FTS_SFX       "_0.fits"         //!< FITS file suffix 
#define FTS_PFX       "%i_"             //!< FITS file prefix 
                                           
// File prefix
#define FTS_PFX_BIAS  'b'               //!< Bias frame
#define FTS_PFX_DARK  'd'               //!< Dark frame
#define FTS_PFX_EXP   'e'               //!< Observation exposure
#define FTS_PFX_FLAT  'f'               //!< Flat frame 
#define FTS_PFX_ACQ   'q'               //!< Acquisition frame
#define FTS_PFX_STD   's'               //!< Standard object
                                           
// Image type written into FITS as OBSTYPE
#define FTS_TYP_BIAS  "BIAS"            //!< Minimum exposure time  
#define FTS_TYP_DARK  "DARK"            //!< Dark
#define FTS_TYP_EXP   "EXPOSE"          //!< Normal   
#define FTS_TYP_FLAT  "SKY-FLAT"        //!< Sky-flat
#define FTS_TYP_ACQ   "ACQUIRE"         //!< Acquisition frame  
#define FTS_TYP_STD   "STANDARD"        //!< Standard object

#define FTS_ID        "123456"          //!< Permissible set of first character in FITS filename
#define FTS_DIR       "."               //!< Default destination         

// Defaults used for testing
#define FTS_OBJ   "Undefined"
#define FTS_RA    "00:00:00.00"
#define FTS_DEC   "00:00:00.00"

// Time related constants
#define TIM_TICK         1000
#define TIM_MILLISECOND  1000.0
#define TIM_MICROSECOND  1000000.0
#define TIM_NANOSECOND   1000000000

// For synchronisation via share memory functions  
#define SHM_RESET      0    //!< Unset state
#define SHM_STABLE     1    //!< Slave temperature stable
#define SHM_ROTATE     2    //!< Master rotation start
#define SHM_TRIG       3    //!< Trigger slave in static mode 
                               
// Camera idenification
#define CAM1     0          //!< Device index. 0 is master
#define CAM2     1          //!< Device index. 1 is slave
#define CAM_ALL -1          //!< Use all cameras

// Camera defaults
#define CAM_COUNT      2     //!< Total number: Prototype = 2
#define CAM_TEMP       4.0   //!< [deg C] Target cooling temperature  
#define CAM_EXP        0.45  //!< [s] Default exposure time 

// Binning 
#define CAM_BIN_1      L"1x1" //!< Binning 1x1
#define CAM_BIN_2      L"2x2" //!< Binning 2x2
#define CAM_BIN_3      L"3x3" //!< Binning 3x3
#define CAM_BIN_4      L"4x4" //!< Binning 4x4
#define CAM_BIN_8      L"8x8" //!< Binning 8x8

// Camera setables
#define CAM_MHZ_100    L"100 MHz" //!< Camera Readout rate 100MHz 
#define CAM_MHZ_270    L"270 MHz" //!< Camera Readout rate 270MHz 
#define IDX_MHZ_100    0          //!< Array index to gain/noise in cam_info
#define IDX_MHZ_270    1          //!< Array index to gain/noise in cam_info 

// Amplifier
#define CAM_AMP_16L    L"16-bit (low noise & high well capacity)"
#define CAM_AMP_12L    L"12-bit (low noise)"  
#define CAM_AMP_12H    L"12-bit (high well capacity)"
#define IDX_AMP_16L    0
#define IDX_AMP_12L    1
#define IDX_AMP_12H    2

// Read direction
#define CAM_RD_BUSEQ   L"Bottom Up Sequential"
#define CAM_RD_BUSIM   L"Bottom Up Simultaneous"
#define CAM_RD_COSIM   L"Centre Out Simultaneous"
#define CAM_RD_OISIM   L"Outside In Simultaneous"
#define CAM_RD_TDSEQ   L"Top Down Sequential"
#define CAM_RD_TDSIM   L"Top Down Simultaneous"

// Transfer encoding
#define CAM_ENC_12     L"Mono12"
#define CAM_ENC_12PACK L"Mono12Packed"
#define CAM_ENC_16     L"Mono16"
#define CAM_ENC_32     L"Mono32"

// Trigger source
#define CAM_TRG_INT    L"Internal" 
#define CAM_TRG_EDGE   L"External" 
#define CAM_TRG_WIDTH  L"External Exposure"
#define CAM_TRG_SW     L"Software" 

// Image 
#define IMG_WIDTH      0     //<! Array index for image dimension - X-axis
#define IMG_HEIGHT     1     //<! Array index for image dimension - Y-axis
#define IMG_DIMENSIONS 2     //!< Numer of dimension on sCMOS chip 
#define IMG_CYCLE      16    //!< Default images per revolution 
#define IMG_TOTAL      IMG_CYCLE * ROT_REVS     
#define MAX_CYCLE      16    //<! Max. images per revolution 
#define MAX_REVS       100   //<! Max. PI stage rotations
#define MAX_IMAGES     MAX_CYCLE * MAX_REVS

#define TEL_UNSET      999.0 //<! Marks unset or invalid telescope FITS parameter

// @brief Common structures _s and templates _t 
typedef struct cam_spec_s
{
    double gain [3];     /// For 3 possible amplifier settings
    double noise[3];
} cam_spec_t;

typedef struct cam_info_s
{
    int     cam_num;          //!< Camera number 
    char   *Filter;           //!< Filter name
    char   *FilterID;         //!< Filter ID 
    double  PolAngle;         //!< Polarisation angle WRT rotator
    AT_WC  *SerialNumber;     //!< Camera internal SN
    AT_WC  *Model;            //!< Camera internal model name
    int    WellDepth;         //!< [e] Pixel well depth
    double DarkCurrent;       //!< Median dark current
    struct cam_spec_s mhz[2]; //!< For 100 & 270 MHz read rates
} cam_info_t;


/// Camera structure. Member names match Andor SDK V3.12 (mostly)
///
typedef struct mop_cam_s
{
    AT_H   Handle;
    AT_WC  SerialNumber[MAX_STR];
    AT_WC  FirmwareVersion[MAX_STR];
    AT_WC *Model;

    int    WellDepth;
    double DarkCurrent;     
    double Gain;            
    double Noise;  
    char  *Filter;
    char  *FilterID;

    AT_U8 *UserBuffer;
    AT_U8 *ImageBuffer[MAX_IMAGES];
    AT_64  ImageSizeBytes;  
    AT_WC  TemperatureStatus[MAX_STR];
    AT_64  SensorWidth;        //!< [px] Sensor width 
    AT_64  SensorHeight;       //!< [px] Sensor height 
    double SensorTemperature;  //!< [C]  Sensor temp 
    double BytesPerPixel; 
    double PixelHeight;
    double PixelWidth;
    long   Dimension[IMG_DIMENSIONS];
    double ReadoutTime; 
    AT_64  TimestampClockFrequency;   //!< Detector timestamp frequency [Hz]
    AT_64  TimestampClock[MAX_IMAGES]; 
    unsigned long Ticks[MAX_IMAGES];
    AT_BOOL FullAOIControl;
    unsigned char *ReturnBuffer[MAX_IMAGES];
    int            ReturnSize  [MAX_IMAGES];

    double ExpReq;             //!< [s] Requested exposure time 
    double ExpVal;             //!< [s] Reported exposure time 
    double ExpMax;             //!< [s] Max Exposure time limit
    double ExpMin;             //!< [s] Min
    double ExpDif;             //!< [s] Total time to acquire image
    double RotReq[MAX_IMAGES]; //!< [deg] Requested rotator position (absolute)
    double RotAng[MAX_IMAGES]; //!< [deg] Actual rotator angle
    double RotEnd[MAX_IMAGES]; //!< [deg] End rotator position for this exposure
    double RotDif[MAX_IMAGES]; //!< [deg] Length of arc for this exposure 
    int    RotN[MAX_IMAGES];   //!< Rotation number 1-100
    int    SeqN[MAX_IMAGES];   //!< Position within rotation 1-8 or 1-16 
    struct timeval ObsStart;
    struct timeval ObsEnd;

    char   id;                 //!< FITS file prefix
    int   seq;                 //!< Sequence 
    char *dir;                 //!< Destination directory
} mop_cam_t;

// Macros
#define btoa(x) ((x)?"true":"false")  /// Boolean to ascii string 

// Function prototypes
// MOPTOP process functions
bool mop_defs( void );                  // Defaults
bool mop_init( void );                  // Init. options
void mop_exit( int exit_code );         // Process exit
bool mop_opts( int argc, char *argv[], char *valid, char *check );// Parse options

// PI rotator function
bool   rot_init( char   *usb, int baud, int timeout, char *trigger );
bool   rot_cmd ( char   *cmd, char *msg );
bool   rot_move( double  angle );              // Start motion 
bool   rot_get ( double *angle );              // Read current position 
bool   rot_set ( double  angle, int timeout ); //  
bool   rot_goto( double  angle, int timeout, double *actual ); // Goto position within timeout 
bool   rot_wait( double  angle, int timeout, bool cw );        // Wait for position within timeout 
bool   rot_trg_ena( bool enable );             // Trigger enable/disable 
bool   rot_ont ( int delay );                  // Wait for on target state
double rot_dbg( char *dbg );                   // Debug: Print & return rotator angle

// Andor camera functions
bool at_chk      ( int ret, char *fn, AT_WC *cmd );
bool at_try      ( mop_cam_t *cam, void *fn, AT_WC *cmd, ...  );
bool cam_init    ( int i );
bool cam_chk     ( mop_cam_t *cam );
bool cam_open    ( mop_cam_t *cam );
bool cam_conf    ( mop_cam_t *cam, double exp );
bool cam_close   ( mop_cam_t *cam );
bool cam_cool    ( mop_cam_t *cam, double TargetTemperature, int timeout, bool fast );
bool cam_queue   ( mop_cam_t *cam );
bool cam_param   ( mop_cam_t *cam );
bool cam_alloc   ( mop_cam_t *cam );                // Allocate memory buffers 
void cam_feature ( mop_cam_t *cam, AT_WC *Feature );// Get feature options
bool cam_acq_circ( mop_cam_t *cam );                // Acquire images - circular buffer
bool cam_acq_stat( mop_cam_t *cam );                // Acquire images -static 
bool cam_acq_ena ( mop_cam_t *cam, AT_BOOL );       // Acquisition enable/disable
bool cam_trg_set ( mop_cam_t *cam, AT_WC *trg );    // Set trigger mode 
bool cam_clk_rst ( mop_cam_t *cam );                // Reset camera clock to zero

// FITS file functions
char *fts_mkname( mop_cam_t *cam, char typ, bool first );
bool  fts_write ( char *filename, mop_cam_t *cam, int seq, int buf );

// Error & logging functions
bool mop_log( bool ret, int level, int fac, char *fmt, ... );
unsigned long cam_ticks( mop_cam_t *cam, int img );

// Utility functions
bool  utl_mksem     ( void        ); // Semaphore
bool  utl_sem_stable( int timeout );
bool  utl_sem_rotate( int timeout );
bool  utl_mkshm     ( void );        // Shared memory
bool  utl_shm_post  ( int state, int which );
bool  utl_shm_wait  ( int state, int which, int timeout, bool reset );
char *utl_arg2msg   ( int   argc, char *argv[], char  *typ );
int   utl_msg2arg   ( char *arg[],char *str,    char **typ );
bool  utl_chk_ip    ( char *ip  );
char *strtoupper    ( char *str );

struct timespec utl_dbl2ts( double dbl );
struct timespec utl_ts_add( struct timespec *t1, struct timespec *t2 );
struct timespec utl_ts_sub( struct timespec *t1, struct timespec *t2 );
int             utl_ts_cmp( struct timespec *t1, struct timespec *t2 );

// Network functions
bool msg_init( char *ip );
bool msg_recv( int timeout, char *recv, int max, int *len, char *chk );
bool msg_send( int timeout, char *send, char *dst, char *recv );
bool msg_chk ( char *msg,   char *chk );

// Filter wheel functions
bool whl_init( int  pos );
bool whl_set ( int  pos );
bool whl_get ( int *pos );


// Include global data and external definitions
#include "mop_dat.h"
