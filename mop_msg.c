/** @file   mop_msg.c
  *
  * @brief  MOPTOP message functions 
  *
  * @author asp 
  *
  * @date   2019-10-14 
  */

#include "mopnet.h" 
#define FAC FAC_MSG

static int                 skt_fd;   // Local UDP socket 
static struct sockaddr_in  skt_adr;  // Local UDP address 


/** @brief       Convert IP:port text into a socket address structure
  *  
  * @param[in]  *ip_port = IP:port as a string 
  *
  * @return      adr     = struct sockaddr_in socket address  
  */
struct sockaddr_in msg_str2adr( char *ip_port )
{
    struct sockaddr_in adr;
    int    port;
    char   ip[32];

//  Zero structure
    memset( &adr, 0, sizeof(adr) ); 

//  Extract IP address and port
    sscanf( ip_port, "%[^:]:%d", ip, &port ); 

//  Populate structure and return 
    adr.sin_family = AF_INET;
    adr.sin_port   = htons( port );
    if ( !inet_aton( ip, &adr.sin_addr ) )
        mop_log( false, LOG_ERR, FAC, "inet_aton() %s", strerror(errno) ); 

    return adr;
}

 
/** @brief       Create and bind UDP socket
  *  
  * @param[in]  *ip_port = IP:port string 
  *
  * @return      true | false = Success | Failure  
  */
bool msg_init( char *ip_port )
{ 
//   Create socket file descriptor 
     if ( (skt_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 )  
     	return mop_log( false, LOG_SYS, FAC, "socket() %s", strerror(errno) ); 

     if (setsockopt(skt_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
     	return mop_log( false, LOG_SYS, FAC, "setsocketop(SO_REUSEADDR) %s", strerror(errno)); 

//   Bind to interface
     skt_adr = msg_str2adr( ip_port );
     skt_adr.sin_addr.s_addr = htonl( INADDR_ANY ); // ... on all network interfaces
     if ( bind( skt_fd, (const struct sockaddr *)&skt_adr, sizeof(skt_adr)) < 0 ) 
         return mop_log( false, LOG_SYS, FAC, "bind() %s", strerror(errno) ); 

     return true; 
}


/** @brief       Receive network message  
  *  
  * @param[in]   timeout = receive timeout [sec], 0=Forever  
  * @param[in]  *msg     = buffer to hold received message 
  * @param[in]   max     = maximum receive length  
  * @param[in]  *len     = variable to hold actual length
  * @param[in]  *exp     = expected message 
  * @param[in]   explen  = expected message length  
  *
  * @return      true | false = Success | Failure  
  */
bool msg_recv( int timeout, char *msg, int max, int *len, char *exp, int explen )
{
    char *ack;        // Acknowledgment string
    bool  ret = true; // Function return

    struct timeval tmo  = {timeout,0};
    struct sockaddr_in adr_rcv; 
    socklen_t len_rcv = sizeof( adr_rcv ); 

//  Set timeout, zero = forever
    if ( setsockopt( skt_fd, SOL_SOCKET, SO_RCVTIMEO, &tmo, sizeof(tmo)) < 0 ) 
        return mop_log( false, LOG_SYS, FAC, "setsockopt() %s", strerror(errno) ); 

//    *len = recvfrom( skt_fd, msg, max, MSG_WAITALL, (struct sockaddr *)&adr_rcv, &len_rcv ); 
    *len = recvfrom( skt_fd, msg, max, 0, (struct sockaddr *)&adr_rcv, &len_rcv ); 
    if ( *len < 0 )
        mop_log( false, LOG_WRN, FAC, "recvfrom() timeout" ); 

//  Got something so nul terminate and check   
    msg[*len] = '\0'; 
    mop_log( true, LOG_INF, FAC, "Received %s", msg ); 

    if ( exp )
    {
//    Check if message is expected type
        if ( ret  = msg_chk( msg, exp, explen ) ) 
            ack = MSG_ACK;
        else
            ack = MSG_NAK;  

//      Send appropriate acknowledgement
        if ( sendto( skt_fd, ack, strlen(ack), MSG_CONFIRM, (const struct sockaddr *)&adr_rcv, len_rcv ) < 0) 
            return mop_log( false, LOG_ERR, FAC, "sendto(%s) %s", ack, strerror(errno)); 
     }

    return ret;
} 


/** @brief       Send network message 
  *  
  * @param[in]   timeout = send timeout [sec] 
  * @param[in]  *send    = message to send (C string) 
  * @param[in]  *dst     = destination 
  * @param[in]  *exp    = reply expected
  *
  * @return      true | false = Success | Failure  
  */
bool msg_send( int timeout, char *send, char *dst, char *exp, int exp_len )
{
    int    len; 
    char   msg_buf[1024];
    struct timeval tmo = {timeout, 0};
    struct sockaddr_in adr_snd = msg_str2adr(dst); 
    struct sockaddr_in adr_rcv; 
    socklen_t len_rcv = sizeof( adr_rcv ); 

//  Set timeout
    if ( setsockopt( skt_fd, SOL_SOCKET, SO_RCVTIMEO, &tmo, sizeof(tmo)) < 0 ) 
        return mop_log( false, LOG_ERR, FAC, "setsockopt() %s", strerror(errno)); 

//  Send message and await ACK
    sendto( skt_fd, send, strlen(send), MSG_DONTWAIT | MSG_DONTROUTE, (const struct sockaddr *)&adr_snd, sizeof(adr_snd)); 

//  
    if ( exp )
    {
        len = recvfrom( skt_fd, msg_buf, sizeof(msg_buf)-1, 0, (struct sockaddr *)&adr_rcv, &len_rcv ); 
        if ( len < 0 )
        {
            return mop_log( false, LOG_WRN, FAC, "recvfrom() timeout" ); 
        }
        else
        {
            msg_buf[len] = '\0'; 
            if ( msg_chk( msg_buf, exp, exp_len ) ) 
                return mop_log( true,  LOG_MSG, FAC, "net_send(%s)", send ); 
            else
                return mop_log( false, LOG_ERR, FAC, "net_send(%s)", send ); 
        }
    }
} 

 
/** @brief       Check received network message matches expected 
  *  
  * @param[in]  *msg    = Received message 
  * @param[in]  *exp    = Expected message 
  * @param[in]  *explen = Expected message length 
  *
  * @return      true | false = Success | Failure  
  */
bool msg_chk( char *msg, char *exp, int explen )
{
   return !strncmp( msg, exp, explen );
}
