/** @file   mop_whl.c
  *
  * @brief  MOPTOP filter wheel functions - CURRENTLY PLACEHOLDER FUNCTIONS ONLY 
  *
  * @author asp 
  *
  * @date   2019-10-24 
  */

#include "mopnet.h"
#define FAC FAC_WHL

/** @brief     Init. filter wheel interface  
  *
  * @param[in] pos     = position to move to 
  * @param[in] timeout = timeout [ms] 
  *
  * @return    true | false = Success | Failure 
  */
bool whl_init( int pos, int timeout )
{
    char buf[256];

    whl_fd = open( whl_dev, O_RDWR|O_NONBLOCK );

    if ( ioctl( whl_fd, HIDIOCGRAWNAME(256), buf ) < 0 )
    {
        return mop_log( false, LOG_ERR, FAC, "whl_init()" );
    }
    else
    {
        mop_log( true,  LOG_DBG, FAC, "Filter wheel name = %s", buf );
        return whl_conf( pos, timeout );
    }
}


/** @brief     Set filter wheel position  
  *
  * @param[in] pos     = position to move to 
  * @param[in] timeout = timeout [ms] 
  *
  * @return    true | false = Success | Failure 
  */
bool whl_conf( int pos, int timeout )
{
    char wbuf[2] = {pos,0}; // Write buffer contains request position
    char rbuf[2] = {0,  0}; // Read buffer indicates actual pos. or moving=0
    int  res;

    int tick  = 10000;      // 10ms tick. Longer than usual as response is slow
    int count = TIM_MICROSECOND * timeout / tick; // Timer count
    
    do 
    {
//      Post a request to get position
        if ( 2 != write( whl_fd, wbuf, 2 ) )
            return mop_log( false, LOG_ERR, FAC, "whl_set(pos=%i)", pos );

//      Wait at least 2ms before checking response 
        usleep(tick);

//      Read position in rbuf[0], 0=moving
        res = read( whl_fd, rbuf, 2 );
        if (( 2   ==  res    )&&  // Read completed
            ( pos == rbuf[0] )  ) // Reached position
        {
            return mop_log( true, LOG_DBG, FAC, "Filter wheel = %i", pos );
        } 
    }
    while ( count-- );
         
    return mop_log( false, LOG_ERR, FAC, "TIMEOUT: whl_set(pos=%i timout=%i)", pos, timeout );
}
