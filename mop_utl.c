/** @file   mop_utl.c
  *
  * @brief  MOPTOP utility functions
  *
  * @author asp 
  *
  * @date   2019-05-01 
  */

#include "mopnet.h"

// Dual process version uses shared memory to synchronise 
#define SHM_NAME "/MOPTOP"
#define SHM_OPTS (O_RDWR | O_CREAT)
#define SHM_PROT (PROT_READ | PROT_WRITE)
#define SHM_MODE (00666 )
int   shm_fd;   // File descriptor for shared memeory
char *shm_ptr;  // Pointer to start of shared data
//char *this_ptr; // Pointer to this process shared data


// Threaded version uses semphore to synchronise
#define SEM_ROTATE "\\ROTATE"
#define SEM_STABLE "\\STABLE"
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
sem_t  *sem_stable;
sem_t  *sem_rotate;


/** @brief     Create inter-process semaphore 
  *
  * @return    true|false = success|failure
  */
bool utl_mksem( void )
{
    if ( SEM_FAILED == (sem_rotate = sem_open(SEM_ROTATE, O_CREAT, SEM_PERMS, 0)))
            return mop_log( false, LOG_SYS, FAC_UTL, "sem_open("SEM_ROTATE")");
    if ( SEM_FAILED == (sem_stable = sem_open(SEM_STABLE, O_CREAT, SEM_PERMS, 0)))
            return mop_log( false, LOG_DBG, FAC_UTL, "sem_open("SEM_STABLE")");

    return true;
}

/** @brief     Wait for other camera to reach stable temperature 
  *
  * @param[in] timeout = timeout [sec] 
  *
  * @return    true | false = Stable | Timeout
  */
bool utl_sem_stable( int timeout )
{
    struct timespec delta;
    struct timespec expire;

//  Set timeout   
    delta.tv_sec  = timeout;
    delta.tv_nsec = 0; 
   
    timespec_get( &expire, TIME_UTC );   
    expire = utl_ts_add( &expire, &delta ); 

    if ( mop_master )
    {
        mop_log( true, LOG_DBG, FAC_UTL, "Master awaiting SEM_STABLE");
        if ( sem_timedwait( sem_stable, &expire ) )
            return mop_log( false, LOG_ERR, FAC_UTL, "Recv SEM_STABLE");
        else
            return mop_log( true,  LOG_DBG, FAC_UTL, "Recv SEM_STABLE");
    }
    else    
    {
        mop_log( true, LOG_DBG, FAC_UTL, "Slave posting SEM_STABLE");
        if ( sem_post( sem_stable ))
            return mop_log( false, LOG_ERR, FAC_UTL, "Post SEM_STABLE");
        else
            return mop_log( true,  LOG_DBG, FAC_UTL, "Post SEM_STABLE"  );
    }
}

/** @brief     Wait for master process to start rotation  
  *
  * @param[in] timeout = timeout [sec] 
  *
  * @return    true | false = Rotating | Timeout
  */
bool utl_sem_rotate( int timeout )
{
    struct timespec delta;
    struct timespec expire;

//  Set timeout   
    delta.tv_sec  = timeout;
    delta.tv_nsec = 0; 

    timespec_get( &expire, TIME_UTC );   
    expire = utl_ts_add( &expire, &delta ); 
  
    if ( mop_master )
    {
        mop_log( true, LOG_DBG, FAC_UTL, "Master posting SEM_ROTATE");
        if ( sem_post( sem_rotate ))
            return mop_log( false, LOG_ERR, FAC_UTL, "Post SEM_ROTATE");
        else
            return mop_log( true,  LOG_DBG, FAC_UTL, "Post SEM_ROTATE");
    }
    else    
    {
        mop_log( true, LOG_DBG, FAC_UTL, "Slave awaiting SEM_ROTATE");
        if ( sem_timedwait( sem_rotate, &expire ) )
            return mop_log( false, LOG_ERR, FAC_UTL, "Slave receive SEM_ROTATE");
        else
            return mop_log( true,  LOG_DBG, FAC_UTL, "Slave receive SEM_ROTATE");
    }
}


/** @brief   Create shared memory for synchronising master/slave process
  *
  * @return  true | false = Success | Failure
  */
bool utl_mkshm( void )
{
    if (((shm_fd  = shm_open( SHM_NAME, SHM_OPTS, SHM_MODE )              ) > 0)&& // Get shared memory file desc.
        ( ftruncate( shm_fd, CAM_COUNT )                                   == 0)&& // Force size
        ((shm_ptr = mmap(NULL, CAM_COUNT, SHM_PROT, MAP_SHARED, shm_fd, 0)) > 0)  )// Map shared memory
    {
//      Zero this process shared memory sync. variable   
//        this_ptr = shm_ptr + this;
       *(shm_ptr + CAM1 ) = SHM_RESET;
       *(shm_ptr + CAM2 ) = SHM_RESET;
        return true; 
    }

    return mop_log( false, LOG_SYS, FAC_UTL, "utl_mkshm()" );
}


/** @brief     Set process' sync. state variable
  *
  * @param[in] state = state to be set 
  * @param[in] which = which process 
  *
  * @return    true 
  */
bool utl_shm_post( int state, int which )
{
    *(shm_ptr + which) = state;
     return true;
}


/** @brief      Wait for a process synchronisation state 
  *
  * @param[in]  state   = flag state to wait for
  * @param[in]  which   = which flag to synchronise on 
  * @param[in]  timeout = timeout [us]
  * @param[in]  reset   = reset flag if state detected 
  *
  * @return    true | false = State reached | Timeout  
  */
bool utl_shm_wait( int state, int which, int timeout, bool reset )
{
    int tick  = TIM_TICK;                          // Timer ticks [us] 
    int count = TIM_MICROSECOND * timeout / tick;  // Timer count [us] 
    char *ptr;

    ptr = shm_ptr + which;   
    do
    { 
        if ( *ptr == state )
        {
           if ( reset )
              *ptr = SHM_RESET;
           return true;
        }
        usleep(tick);
    } while( count-- );
    return mop_log( false, LOG_ERR, FAC_UTL, "utl_shm_wait() timeout");
}


/** @brief     Convert a decimal time [sec] to timespec
  *
  * @param[in] dbl = input time as decimal seconds
  *
  * @return    ts  = timespec struct   
  */
struct timespec utl_dbl2ts( double dbl )
{
    static struct timespec ts;
    double sec;

    ts.tv_nsec = TIM_NANOSECOND * modf( dbl, &sec );  
    ts.tv_sec  = (int)sec;

    return ts;
}


/** @brief      Add two timespec values t1 + t2
  *
  * @param[in] *t1 = time 1
  * @param[in] *t2 = time 2
  *
  * @return     sum = sum of t1 + t2 in a timespec struct   
  */
struct timespec utl_ts_add( struct timespec *t1, struct timespec *t2 )
{
    static struct timespec sum;

    sum.tv_sec  = t1->tv_sec  + t2->tv_sec;
    sum.tv_nsec = t1->tv_nsec + t2->tv_nsec;

//  If tv_nsec overflowed apply correction
    if ( sum.tv_nsec >= TIM_NANOSECOND )
    {
       sum.tv_sec++;
       sum.tv_nsec -= TIM_NANOSECOND; 
    }  

    return sum;
} 


/** @brief  Subtract two timespec values t1 - t2
  *
  * @param[in] *t1 = time 1
  * @param[in] *t2 = time 2
  *
  * @return    diff = difference in a  timespec struct   
  */
struct timespec utl_ts_sub( struct timespec *t1, struct timespec *t2 )
{
    static struct timespec diff = {0, 0};

    if (( t1->tv_sec  < t2->tv_sec                                 )||
        ((t1->tv_sec == t2->tv_sec) && (t1->tv_nsec <= t2->tv_nsec))  )
    {
//      Invalid or identical subtraction. Return {0,0};
        return diff;
    }
    else
    {
        diff.tv_sec = t1->tv_sec - t2->tv_sec;
        if ( t1->tv_nsec <  t2->tv_nsec )
        {
            diff.tv_nsec = TIM_NANOSECOND + t1->tv_nsec - t2->tv_nsec;
            diff.tv_sec--;
        }
        else
        {
            diff.tv_nsec = t1->tv_nsec - t2->tv_nsec;
        }
    }

    return diff;
}


/** @brief Compare timespec times
  *
  * @param[in] *t1 = time 1
  * @param[in] *t2 = time 2
  *
  * @return    -1 = less than, 0 = same, +1 = greater than 
  */
int utl_ts_cmp( struct timespec *t1, struct timespec *t2 )
{
    if      ( t1->tv_sec  < t2->tv_sec )  
        return -1;                     // Less than
    else if ( t1->tv_sec  > t2->tv_sec )
        return  1;                     // Greater than
    else if ( t1->tv_nsec < t2->tv_nsec)
        return -1;                     // Less than
    else if ( t1->tv_nsec > t2->tv_nsec)
        return  1;                     // Greater than
    else
        return  0;                     // Equal to 
}


/** @brief     Convert programme arguments into a network message string. 
  *
  * @param[in] argc = argument count 
  * @param[in] argv = pointer array to argument variables  
  * @param[in] typ  = prefix identifying network message type
  *
  * @return    str  = pointer to final string (local static storage)  
  */
char *utl_arg2msg( int argc, char *argv[], char *typ )
{
    int c;            
    static char str[MAX_STR];
    char       *ptr = str; 

//  Prefix  message type if provided 
    if ( typ ) 
        ptr += sprintf( ptr, "%s", typ );

    opterr = 0; // Suppress errors
    optind = 1; // Set index to start

//  Parse valid options to be added to string
    while (( c = getopt( argc, argv, OPTS_MSG )) != -1 )
        if ( c == '?' )
            ptr += sprintf( ptr, " -%c"  , c );
        else
            ptr += sprintf( ptr, " -%c%s", c, optarg?optarg:"" );

//  nul terminate return string
    *ptr = '\0';

    return str;
}


/** @brief      Convert message string contents into programme argument 
  *
  * @param[in]  argv = pointer to array of pointers to argument strings
  * @param[in]  str  = input string to be converted
  * @param[out] typ  = network message type   
  *
  * @return     argc = argument count  
  */
int utl_msg2arg( char **argv, char *str, char **typ )
{
    int argc = 0;

//  Fake process name is first item in argument list
    argv[argc++]="./mopnet";

//  First field should be the message type
    *typ = strtok( str, " " );

//  Load subsequent fields into array
    while ((argv[argc] = strtok( NULL, " " )))
        argc++;

    return argc;
}


/** @brief     Uppercase a string in-situ
  *
  * @param[in] *str = pointer to input string 
  *
  * @return    *str = pointer to converted input string
  */
char *strtoupper( char *str )
{
    char *ret;

    for( ret = str ; *str; str++ )
        *str = toupper(*str);
    return ret;
}


/** @brief      Check IP address format = n.n.n.n:port is valid-ish
  *
  * @param[in] *ip = pointer to input string 
  *
  * @return     true | false = Success | Failure 
  */
bool utl_chk_ip( char *ip )
{
    char         c;    // Check for extra field at end of input
    unsigned int h[6]; // hex digits  0-255
    unsigned int p;    // port number 0-65535 although anything below 1024 is "naughty"
    return ((5==sscanf(ip,"%u.%u.%u.%u:%u%c",&h[0],&h[1],&h[2],&h[3],&p,&c))&&  // Must be 5 fields
            (h[0] <256 && h[1] <256 && h[2] <256 && h[3] <256 && p <65535  )  );// Within limits
}
