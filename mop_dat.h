/** @file   mop_dat.h
  *
  * @brief  MOPTOP global variables. Data declared within mopnet.c, external everywhere else.
  * 
  * @author asp 
  * 
  * @date   2019-05-01 
  */
#ifdef MAIN

/*
 * Look-up tables and associated #defines 
 */

// Used to convert ANDOR return errors into text
const char *at_erray[] =
{
    "AT_SUCCESS",                     //0
    "AT_ERR_NOTINITIALISED",          //1
    "AT_ERR_NOTIMPLEMENTED",          //2
    "AT_ERR_READONLY",                //3
    "AT_ERR_NOTREADABLE",             //4
    "AT_ERR_NOTWRITABLE",             //5
    "AT_ERR_OUTOFRANGE",              //6
    "AT_ERR_INDEXNOTAVAILABLE",       //7
    "AT_ERR_INDEXNOTIMPLEMENTED",     //8
    "AT_ERR_EXCEEDEDMAXSTRINGLENGTH", //9
    "AT_ERR_CONNECTION",              //10
    "AT_ERR_NODATA",                  //11
    "AT_ERR_INVALIDHANDLE",           //12
    "AT_ERR_TIMEDOUT",                //13
    "AT_ERR_BUFFERFULL",              //14
    "AT_ERR_INVALIDSIZE",             //15
    "AT_ERR_INVALIDALIGNMENT",        //16
    "AT_ERR_COMM",                    //17
    "AT_ERR_STRINGNOTAVAILABLE",      //18
    "AT_ERR_STRINGNOTIMPLEMENTED",    //19
    "AT_ERR_NULL_FEATURE",            //20
    "AT_ERR_NULL_HANDLE",             //21
    "AT_ERR_NULL_IMPLEMENTED_VAR",    //22
    "AT_ERR_NULL_READABLE_VAR",       //23
    "AT_ERR_NULL_READONLY_VAR",       //24
    "AT_ERR_NULL_WRITABLE_VAR",       //25
    "AT_ERR_NULL_MINVALUE",           //26
    "AT_ERR_NULL_MAXVALUE",           //27
    "AT_ERR_NULL_VALUE",              //28
    "AT_ERR_NULL_STRING",             //29
    "AT_ERR_NULL_COUNT_VAR",          //30
    "AT_ERR_NULL_ISAVAILABLE_VAR",    //31
    "AT_ERR_NULL_MAXSTRINGLENGTH",    //32
    "AT_ERR_NULL_EVCALLBACK",         //33
    "AT_ERR_NULL_QUEUE_PTR",          //34
    "AT_ERR_NULL_WAIT_PTR",           //35
    "AT_ERR_NULL_PTRSIZE",            //36
    "AT_ERR_NOMEMORY,"                //37
    "AT_ERR_DEVICEINUSE",             //38
    "AT_ERR_DEVICENOTFOUND",          //39
};

// Converts ANDOR utility return values to text
const char *ut_erray[] =
{
    "AT_ERR_1000",                       // 1000
    "AT_ERR_1001",                       // 1001
    "AT_ERR_INVALIDOUTPUTPIXELENCODING", // 1002
    "AT_ERR_INVALIDINPUTPIXELENCODING",  // 1003
    "AT_ERR_INVALIDMETADATAINFO",        // 1004
    "AT_ERR_CORRUPTEDMETADATA",          // 1005
    "AT_ERR_METADATANOTFOUND"            // 1006
};

// Colour codes for text output
const char *log_colour[] =
{
    COL_NULL,  // NONE - Suppress all 
    COL_RED,   // CRIT - Critical error
    COL_RED,   // SYS  - Unrecoverable system error
    COL_RED,   // ERR  - Unrecoverable application
    COL_YELLOW,// WRN  - Recoverable/retry warning 
    COL_NULL,  // IMG  - Image acquisition 
    COL_NULL,  // INF  - Informational 
    COL_BLUE,  // MSG  - Messaging 
    COL_GREEN, // DBG  - Debug
    COL_NULL,  // CMD  - Default 
}; 


// Filter wheel colour names
const char *whl_colour[] =
{
    "?",              // Error/Unknown
    "Baader L",       // 
    "Baader R",       // Red
    "Baader G",       // Green
    "Baader B",       // Blue
    "RG-695 Longpass" // Longpass
};


// Camera information from Andor spec. sheet plus filter / orientation
const cam_info_t cam_info[CAM_COUNT] =
{
  {
    .Filter   = "",
    .FilterID = "",
    .PolAngle = 0.0,
    .SerialNumber= L"VSC-04181",
    .Model       = L"ZYLA-4.2P-USB3",
    .WellDepth   = 32241,   // e 
    .DarkCurrent = 0.1080,  // e / pixel /sec
    .mhz[IDX_MHZ_100].gain [IDX_AMP_16L] = 0.55, // e / ADU
    .mhz[IDX_MHZ_100].gain [IDX_AMP_12L] = 0.27,
    .mhz[IDX_MHZ_100].gain [IDX_AMP_12H] = 8.48,
    .mhz[IDX_MHZ_270].gain [IDX_AMP_16L] = 0.54,
    .mhz[IDX_MHZ_270].gain [IDX_AMP_12L] = 0.29,
    .mhz[IDX_MHZ_270].gain [IDX_AMP_12H] = 8.44,
    .mhz[IDX_MHZ_100].noise[IDX_AMP_16L] = 1.09, // e (RMS)
    .mhz[IDX_MHZ_100].noise[IDX_AMP_12L] = 0.90,
    .mhz[IDX_MHZ_100].noise[IDX_AMP_12H] = 7.42,
    .mhz[IDX_MHZ_270].noise[IDX_AMP_16L] = 1.31,
    .mhz[IDX_MHZ_270].noise[IDX_AMP_12L] = 1.09,
    .mhz[IDX_MHZ_270].noise[IDX_AMP_12H] = 7.16 
  },{
    .Filter   = "",
    .FilterID = "",
    .PolAngle = 90.0,
    .SerialNumber= L"VSC-04151",
    .Model       = L"ZYLA-4.2P-USB3",
    .WellDepth   = 32699,
    .DarkCurrent = 0.1063,
    .mhz[IDX_MHZ_100].gain [IDX_AMP_16L] = 0.54,
    .mhz[IDX_MHZ_100].gain [IDX_AMP_12L] = 0.26,
    .mhz[IDX_MHZ_100].gain [IDX_AMP_12H] = 8.35,
    .mhz[IDX_MHZ_270].gain [IDX_AMP_16L] = 0.53,
    .mhz[IDX_MHZ_270].gain [IDX_AMP_12L] = 0.29,
    .mhz[IDX_MHZ_270].gain [IDX_AMP_12H] = 8.33,
    .mhz[IDX_MHZ_100].noise[IDX_AMP_16L] = 1.11,
    .mhz[IDX_MHZ_100].noise[IDX_AMP_12L] = 0.89,
    .mhz[IDX_MHZ_100].noise[IDX_AMP_12H] = 7.35,
    .mhz[IDX_MHZ_270].noise[IDX_AMP_16L] = 1.32,
    .mhz[IDX_MHZ_270].noise[IDX_AMP_12L] = 1.11,
    .mhz[IDX_MHZ_270].noise[IDX_AMP_12H] = 7.02 
  }
};

// Facility names, NUL is unused. Must match FAC order in mopnet.h
const char *fac_levels[] = {"NUL","MOP","LOG","UTL","OPT","CAM","ROT","FTS","MSG","WHL","CMD"}; 

// In decreasing order of severity as used for debug level, Must match LOG order in mopnet.h
const char *log_levels[] = {"NONE","CRIT","SYS","ERR","WRN","IMG","INF","MSG","DBG","CMD"}; 

int       log_level   = LOG_WRN;     // Default log level
char      log_pfx[MAX_STR];          // Prefix log lines with this text (debug)
char     *log_file    = NULL;        // Log file name 
FILE     *log_fp      = NULL;        // Log file handle

bool      one_cam     = false;       // DEBUG: Single camera 
bool      mop_master  = false;       // Master=true, Otherwise slave=false
bool      mop_quiet   = false;       // Suppress screen output
int       mop_cams    = CAM_COUNT;   // Number of cameras 
FILE     *mop_log_fp;                // Log file pointer
mop_cam_t mop_cam;         
char     *mop_proc    = "";          // Process name
bool      mop_kill    = false;       // Kill flag (used by command process 
                                     
int       rot_id      =  1;          // Rotator device ID (fixed)
char     *rot_axis    = "1";         // Rotator axis ID   (fixed)
double    rot_zero    = ROT_ZERO;    // Rotator zero position
double    rot_vel     = ROT_VEL;     // Rotator velocity [degree/sec] 
double    rot_stp     = ROT_STP16;   // Rotator increment [degrees]
int       rot_revs    = ROT_REVS;    // Rotator revolutions
double    rot_final;                 // Rotator final position
char     *rot_usb     = ROT_USB;     // Rotator USB device
int       rot_sign    = ROT_CW;      // Rotator direction 1=CW, 0=static, -1=CCW
                                     
int       cam_num     = 1;           // Camera number <1-2>
int       cam_idx     = 0;           // Andor's camera index number <0-1>
bool      cam_quick   = true;        // Fast cooling (skips temperature stability state)
bool      cam_auto    = true;        // Use automatic exposure
double    cam_exp     = CAM_EXP;     // [s] Camera exposure time 
double    cam_temp    = CAM_TEMP;    // [deg C] Camera target temperature
wchar_t  *cam_mhz     = CAM_MHZ_100; // [MHz] PixelReadOutRate
wchar_t  *cam_amp     = CAM_AMP_16L; // SimplePreAmpGainControl, bits/noise
wchar_t  *cam_enc     = CAM_ENC_16;  // Pixel encoding, bits/packed 
wchar_t  *cam_trg     = CAM_TRG_EDGE;// Camera trigger mode
wchar_t  *cam_rd      = CAM_RD_OISIM;// Camera read direction
wchar_t  *cam_bin     = CAM_BIN_2;   // Camera binning 

int       img_bin     = 2;	     // Image binning - Must match camera binning (cam_bin)
int       img_total   = IMG_TOTAL;   // Total number of images 
int       img_cycle   = IMG_CYCLE;   // Images per revolution
AT_U8    *ImageBuffer = NULL;        // Monolithic memory allocation pointer
AT_U8    *img_mono16  = NULL;        // Buffer containing image converted to 16-bit monochrome
AT_64     img_mono16size = 0;        // Size of mono16 alloc'ed buffer

int       fts_ccdxbin = 2;           // X binning NOTE: Must equal img_bin
int       fts_ccdybin = 2;           // Y binning NOTE: Must equal img_bin
char      fts_id[]    = FTS_ID;      // Permitted list of FITS IDs (first char of filename)
bool      fts_sync    = true;        // Write files immediately after acquisition
char      fts_pfx     = FTS_PFX_EXP; // Exposure code prefix
char     *fts_typ     = FTS_TYP_EXP; // Exposure type  
char      fts_obj[MAX_STR];          // Object name
char      fts_ra [MAX_STR];          // Object RA
char      fts_dec[MAX_STR];          // Object DEC
char      fts_dir[MAX_STR];          // Destination for output files
int       fts_run;                   // Run number

double    tel_foc     = TEL_UNSET;   // Telescope parameters for FITS file header
double    tel_cas     = TEL_UNSET;
double    tel_alt     = TEL_UNSET;
double    tel_azm     = TEL_UNSET;   

int       whl_pos     = 5;           // Default filter wheel position  
int       whl_fd      = 0;           // Filter wheel file descriptor 
char     *whl_dev     = WHL_DEV;     // Filter wheel device

char     *ipmaster    = IPMASTER;    // Master  IP address xx.xx.xx.xx.xx.xx:port
char     *ipslave     = IPSLAVE;     // Slave   IP address xx.xx.xx.xx.xx.xx:port  
char     *ipcommand   = IPCOMMAND;   // Command IP address xx.xx.xx.xx.xx.xx:port  
#else
extern char    *at_erray[];
extern char    *ut_erray[];
extern char    *log_colour[];
extern char    *whl_colour[];

extern char    *fac_levels[]; 
extern char    *log_levels[];
extern int      log_level;
extern char     log_pfx[MAX_STR];
extern char    *log_file;
extern FILE    *log_fp;

extern bool     one_cam;
extern bool     mop_master;
extern int      mop_inst;
extern bool     mop_quiet;
extern int      mop_which;
extern int      mop_count;
extern int      mop_cams;
extern FILE    *mop_log_fp;
extern mop_cam_t mop_cam;
extern char    *mop_proc;
extern bool     mop_kill; 

extern int      rot_id;   
extern char    *rot_axis;
extern double   rot_zero; 
extern double   rot_vel; 
extern double   rot_stp;
extern int      rot_revs;
extern double   rot_final;
extern char    *rot_usb;
extern int      rot_sign;

extern int      cam_num;
extern int      cam_idx;
extern bool     cam_quick;
extern bool     cam_auto;
extern double   cam_exp; 
extern double   cam_temp;
extern wchar_t *cam_mhz;
extern wchar_t *cam_amp;
extern wchar_t *cam_enc;
extern wchar_t *cam_trg; 
extern cam_info_t cam_info[CAM_COUNT];
extern wchar_t *cam_bin;
extern wchar_t *cam_rd;

extern int      img_total;
extern int      img_cycle;
extern AT_U8   *ImageBuffer;
extern AT_U8   *img_mono16;
extern AT_64    img_mono16size;
extern int      img_bin;

extern char     fts_id[];
extern bool     fts_sync;
extern char     fts_pfx;
extern char    *fts_typ;
extern char     fts_obj[MAX_STR];
extern char     fts_ra [MAX_STR];
extern char     fts_dec[MAX_STR];
extern char     fts_dir[MAX_STR];
extern int      fts_ccdxbin;
extern int      fts_ccdybin;
extern int      fts_run;

extern double   tel_foc;
extern double   tel_cas;
extern double   tel_alt;
extern double   tel_azm;

extern int      whl_pos; 
extern char    *whl_dev;
extern char     whl_fd;

extern char    *ipmaster;
extern char    *ipslave; 
extern char    *ipcommand;  
#endif
