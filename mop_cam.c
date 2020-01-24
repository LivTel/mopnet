/** @file   mop_cam.c
  *
  * @brief MOPTOP Andor Zyla camera access and control functions  
  *
  * @author asp 
  *
  * @date   2019-07-09 
  */

#include "mopnet.h"
#define FAC FAC_CAM

/** @brief     Check Andor API AT_*() function return value
  *
  * @param[in]  ret = AT_ function return value to be checked
  * @param[in] *fn  = AT_ function name (C string) 
  * @param[in] *cmd = AT_ sub-command name (unicode string) 
  *
  * @return    true | false = Success | Failure
  */
bool at_chk( int ret, char *fn, AT_WC *cmd  )
{
    if ( !ret )
        return mop_log( true,  LOG_DBG, FAC, "AT_%s(%ls) OK. Ret=%i", fn, cmd, ret );
    else if ( ret >= AT_ERR_MIN && ret <= AT_ERR_MAX )
        return mop_log( false, LOG_ERR, FAC, "AT_%s(%ls) fail. Ret=%i Err=%s", fn, cmd, ret, at_erray[ret - AT_ERR_MIN]);
    else if ( ret >= UT_ERR_MIN && ret <= UT_ERR_MAX )
        return mop_log( false, LOG_ERR, FAC, "AT_%s(%ls) fail. Ret=%i Err=%s", fn, cmd, ret, ut_erray[ret - UT_ERR_MIN]);
    else
        return mop_log( false, LOG_ERR, FAC, "AT_%s(%ls) fail. Ret=%i Err=%s", fn, cmd, ret, "ERR_UNKNOWN" );
}

/** @brief     Call an AT_ function and retry on failure
  *
  * @param[in] *cam = pointer to camera info structure 
  * @param[in] *fn  = AT_ function to be called (address of function) 
  * @param[in] *cmd = function sub-commands (unicode string) 
  * @param[in] ...  = additional sub-command parameters 
  *
  * @return    true | false = Success | Failure
  */
bool at_try( mop_cam_t *cam, void *fn, AT_WC *cmd, ...  )
{
    bool ok = false; // Was AT_ function call successful
    va_list val;     // List of parameters to be passed to AT function

//  Re-try loop
    for ( int try = 1; try <= AT_TRY; try++ )
    {
//      Init. variable arg list each time through loop
        va_start( val, cmd );

/*  Yikes! This needs some explaining.
 *  The function address is passed in and identified using "if" because switch/case can't be used.  
 *  Apart from AT_Open, the camera handle is passed to the function.
 *  cmd is the sub-function to be invoked and is passed as a unicode string
 *  To handle variable types, any parameter is extracted using va_arg() with appropriate type.
 *  Read strings are fixed at maximum length of 255 bytes
 *  The last 2 string params are passed onto at_chk() for debug logging   
 */
//                     Function         Fn parameters                     Debug Fn name & sub-command  
        if      (fn == AT_Open         )         
            ok=at_chk( AT_Open         (cam_idx,    &cam->Handle              ),"Open"         ,cmd);
        else if (fn == AT_SetBool      )      
            ok=at_chk( AT_SetBool      (cam->Handle, cmd, va_arg(val,AT_BOOL )),"SetBool"      ,cmd);
        else if (fn == AT_SetEnumString)
            ok=at_chk( AT_SetEnumString(cam->Handle, cmd, va_arg(val,AT_WC  *)),"SetEnumString",cmd);
        else if (fn == AT_SetFloat     )     
            ok=at_chk( AT_SetFloat     (cam->Handle, cmd, va_arg(val,double  )),"SetFloat"     ,cmd);
        else if (fn == AT_GetBool      )     
            ok=at_chk( AT_GetBool      (cam->Handle, cmd, va_arg(val,AT_BOOL*)),"GetBool"      ,cmd);
        else if (fn == AT_GetString    )    
            ok=at_chk( AT_GetString    (cam->Handle, cmd, va_arg(val,AT_WC  *),MAX_STR-1),"GetString",cmd);
        else if (fn == AT_GetFloat     )     
            ok=at_chk( AT_GetFloat     (cam->Handle, cmd, va_arg(val,double *)),"GetFloat"     ,cmd);
        else if (fn == AT_GetInt       )      
            ok=at_chk( AT_GetInt       (cam->Handle, cmd, va_arg(val,AT_64  *)),"GetInt"       ,cmd);
        else if (fn == AT_GetFloatMin  ) 
            ok=at_chk( AT_GetFloatMin  (cam->Handle, cmd, va_arg(val,double *)),"GetFloatMin"  ,cmd);
        else if (fn == AT_GetFloatMax  ) 
            ok=at_chk( AT_GetFloatMax  (cam->Handle, cmd, va_arg(val,double *)),"GetFloatMax"  ,cmd);
        else if (fn == AT_Flush        )        
            ok=at_chk( AT_Flush        (cam->Handle                           ),"Flush"        ,cmd);  
        else if (fn == AT_Close        )        
            ok=at_chk( AT_Close        (cam->Handle                           ),"Close"        ,cmd);  
        else if (fn == AT_Command      )        
            ok=at_chk( AT_Command      (cam->Handle, cmd                      ),"Command"      ,cmd);  
        else if (fn == AT_IsReadOnly   )        
            ok=at_chk( AT_IsReadOnly   (cam->Handle, cmd, va_arg(val,AT_BOOL*)),"IsReadOnly"   ,cmd);  
        else if (fn == AT_GetEnumIndex )
            ok=at_chk( AT_GetEnumIndex (cam->Handle, cmd, va_arg(val,int    *)),"GetEnumIndex" ,cmd); 
        else if (fn == AT_GetEnumStringByIndex)
            ok=at_chk( AT_GetEnumStringByIndex(cam->Handle, cmd, va_arg(val,int), va_arg(val, AT_WC *), MAX_STR-1),"GetEnumStringByIndex", cmd);
        else
            mop_log( false, LOG_ERR, FAC, "Unrecognised AT_ function" ); 

//      Did the function complete successfully?
        if ( ok ) 
        {
            va_end( val );
            return true;
        } 
        else
        {
            mop_log( true, LOG_WRN, FAC, "retry %i ...", try ); 
            sleep( AT_DLY );
        }
    }

   va_end( val );
   return false;
}


/** @brief     Camera initialisation 
  *
  * @param[in] i = index to FITS ID lookup table 
  * 
  * @return    true | false = Success | Failure
  */
bool cam_init( int i )
{
//  Initialise camera defaults that are not easily settable in mopdat.h    
    memset( &mop_cam, 0, sizeof(mop_cam));
    mop_cam.id       = fts_id[i];
    mop_cam.Handle   = AT_HANDLE_UNINITIALISED;

//  Set FTS binning to match camera image
    fts_ccdxbin = fts_ccdybin = img_bin;

//  Init. Andor camera and conversion utility libraries
    return (at_chk( AT_InitialiseLibrary(),       "InitialiseLibrary"       , L"")&&
            at_chk( AT_InitialiseUtilityLibrary(),"InitialiseUtilityLibrary", L"")  );
}


/** @brief     Tidy-up camera data before exiting process  
  *
  * @param[in] *cam = pointer to camera info structure 
  *
  * @return    true | false = Success | Failure
  */
bool cam_close( mop_cam_t *cam )
{
    if ( cam->Handle )
    {
//      Inhibit image acquisition
        cam_acq_ena( cam, AT_FALSE   );
        cam_trg_set( cam, CAM_TRG_SW );

//      Flush and close camera 
        at_try(cam, AT_Flush, L"", NULL );
        at_try(cam, AT_Close, L"", NULL );
    }

//  Release output buffer memory 
    if ( img_mono16 )
    {
        free( img_mono16 );
        img_mono16 = NULL;
    }

//  Release circular buffer memory 
    for ( int i = 0; i < MAX_CYCLE; i++ )
        if ( cam->ImageBuffer[i] )
        { 
           free( cam->ImageBuffer[i] );
           cam->ImageBuffer[i] = NULL;
        }

//  Close AT libraries
    return at_chk( AT_FinaliseLibrary()       ,"FinaliseLibrary"       , L"")&&
           at_chk( AT_FinaliseUtilityLibrary(),"FinaliseUtilityLibrary", L"")  ;
}


/** @brief     Ensure cooling is enabled and wait for temperature to stabilise below target
  *
  * @param[in] *cam              = pointer to camera info structure
  * @param[in] TargetTemperature = [deg C] to reach before returning
  * @param[in] timeout           = [sec] max. cooling time 
  * @param[in] fast              = true = Wait for target temp., false = Wait for stable temp. 
  *
  * @return    true | false = Success | Failure
  */
bool cam_cool( mop_cam_t *cam, double TargetTemperature, int timeout, bool fast ) 
{
    int TempStatusIndex;
    
//  Wait for camera to report stable temperature
    if ( !fast )
        do 
        {
            sleep(1);
            at_try( cam, AT_GetFloat            ,L"SensorTemperature", &cam->SensorTemperature );
            at_try( cam, AT_GetEnumIndex        ,L"TemperatureStatus", &TempStatusIndex        );
            at_try( cam, AT_GetEnumStringByIndex,L"TemperatureStatus", cam->TemperatureStatus, TempStatusIndex        );
            mop_log( true, LOG_INF, FAC, "Thermal=%ls T=%-6.2fC" , cam->TemperatureStatus, cam->SensorTemperature );
        } while( wcscmp( L"Stabilised", cam->TemperatureStatus) && ( timeout-- > 0 ) );

//  Wait for target temperature to be reached
    do 
    {
        sleep(1);
        at_try( cam, AT_GetFloat            ,L"SensorTemperature", &cam->SensorTemperature );
        at_try( cam, AT_GetEnumIndex        ,L"TemperatureStatus", &TempStatusIndex        );
        at_try( cam, AT_GetEnumStringByIndex,L"TemperatureStatus", cam->TemperatureStatus, TempStatusIndex        );
        mop_log( true, LOG_INF, FAC, "Thermal=%ls T=%-6.2fC" , cam->TemperatureStatus, cam->SensorTemperature );
    } while ( ( cam->SensorTemperature > TargetTemperature ) && ( timeout-- > 0 ) ); 

//  Was target temperature reached within timeout? 
    if ( timeout <= 0 )
    	return mop_log( false, LOG_WRN, FAC, "Cooling Timeout" );
    else
        return mop_log( true,  LOG_INF, FAC, "Thermal=%ls T=%-6.2fC < %-6.2f", 
                        cam->TemperatureStatus, cam->SensorTemperature, TargetTemperature );
}


/** @brief      DEBUG ONLY: Checks camera parameter's readonly status    
  *
  * @param[in] *cam = pointer to camera info structure 
  *
  * @return     true 
  */
bool cam_chk( mop_cam_t *cam )
{
    AT_BOOL ReadOnly;
    
    at_try( cam, AT_IsReadOnly, L"Framecount", &ReadOnly );
    mop_log( ReadOnly, LOG_DBG, FAC, "FrameCount Readonly=%i", ReadOnly );

    at_try( cam, AT_IsReadOnly, L"AccumulateCount", &ReadOnly );
    mop_log( ReadOnly, LOG_DBG, FAC, "AccumulateCount Readonly=%i", ReadOnly );

    return true;
}


/** @brief      Open a camera handle 
  *
  * @param[in] *cam = Pointer to camera info structure
  *
  * @return     true | false = Success | Failure
  */
bool cam_open( mop_cam_t *cam )
{
     return at_try(cam, (void*)AT_Open, L"", &cam->Handle );
}


/** @brief     Configure camera and get settings
  *
  * @param[in] *cam = pointer to camera structure
  * @param[in]  exp = [sec] exposure time
  *
  * @return    true | false = Success | Failure
  */
bool cam_conf( mop_cam_t *cam, double exp )
{
     if (at_try(cam,(void*)AT_SetBool      ,L"SensorCooling"            ,AT_TRUE                      )&&
         at_try(cam,(void*)AT_SetBool      ,L"MetadataEnable"           ,AT_TRUE                      )&&
         at_try(cam,(void*)AT_SetBool      ,L"MetadataTimestamp"        ,AT_TRUE                      )&&
         at_try(cam,(void*)AT_SetBool      ,L"SpuriousNoiseFilter"      ,AT_FALSE                     )&& 
         at_try(cam,(void*)AT_SetBool      ,L"StaticBlemishCorrection"  ,AT_FALSE                     )&&
         at_try(cam,(void*)AT_SetBool      ,L"RollingShutterGlobalClear",AT_TRUE                      )&&
         at_try(cam,(void*)AT_SetEnumString,L"ElectronicShutteringMode" ,L"Rolling"                   )&&
         at_try(cam,(void*)AT_SetEnumString,L"SensorReadoutMode"        ,cam_rd                       )&&
         at_try(cam,(void*)AT_SetEnumString,L"SimplePreAmpGainControl"  ,cam_amp                      )&&
         at_try(cam,(void*)AT_SetEnumString,L"PixelEncoding"            ,cam_enc                      )&&
         at_try(cam,(void*)AT_SetEnumString,L"PixelReadoutRate"         ,cam_mhz                      )&&
         at_try(cam,(void*)AT_SetEnumString,L"CycleMode"                ,L"Continuous"                )&&
         at_try(cam,(void*)AT_SetEnumString,L"AOIBinning"               ,cam_bin                      )&&
         at_try(cam,(void*)AT_SetEnumString,L"TriggerMode"              ,cam_trg                      )&&
         at_try(cam,(void*)AT_SetFloat     ,L"ExposureTime"             ,exp                          )&&
         at_try(cam,(void*)AT_GetBool      ,L"FullAOIControl"           ,&cam->FullAOIControl         )&&
         at_try(cam,(void*)AT_GetString    ,L"SerialNumber"             ,cam->SerialNumber            )&&
         at_try(cam,(void*)AT_GetString    ,L"FirmwareVersion"          ,cam->FirmwareVersion         )&&
         at_try(cam,(void*)AT_GetFloat     ,L"ExposureTime"             ,&cam->ExpVal                 )&&
         at_try(cam,(void*)AT_GetFloat     ,L"ReadoutTime"              ,&cam->ReadoutTime            )&&
         at_try(cam,(void*)AT_GetFloat     ,L"BytesPerPixel"            ,&cam->BytesPerPixel          )&&
         at_try(cam,(void*)AT_GetFloat     ,L"PixelWidth"               ,&cam->PixelWidth             )&&
         at_try(cam,(void*)AT_GetFloat     ,L"PixelHeight"              ,&cam->PixelHeight            )&&
         at_try(cam,(void*)AT_GetInt       ,L"SensorWidth"              ,&cam->SensorWidth            )&&
         at_try(cam,(void*)AT_GetInt       ,L"SensorHeight"             ,&cam->SensorHeight           )&&
         at_try(cam,(void*)AT_GetInt       ,L"TimestampClockFrequency"  ,&cam->TimestampClockFrequency)&&
         at_try(cam,(void*)AT_GetInt       ,L"ImageSizeBytes"           ,&cam->ImageSizeBytes         )&&
         at_try(cam,(void*)AT_GetFloatMin  ,L"ExposureTime"             ,&cam->ExpMin                 )&&
         at_try(cam,(void*)AT_GetFloatMax  ,L"ExposureTime"             ,&cam->ExpMax                 )&&
         at_try(cam,(void*)AT_Flush        ,L""                         ,NULL                         )  )
     {              
//       If automatic exposure then evaluate a time for current camera settings
         if ( cam_auto &&   // Use auto exposure 
              rot_sign    ) // and not static
         {
             cam_exp = exp = fabs((rot_stp / rot_vel)) - 2.0 * cam->ReadoutTime;
             at_try(cam, AT_SetFloat, L"ExposureTime", exp         );
             at_try(cam, AT_GetFloat, L"ExposureTime", &cam->ExpVal);
             mop_log( true, LOG_INF, FAC, "Automatic exposure = %fs", cam->ExpVal ); 
         }
     
//       Re-get actual exposure in case it was rounded up/down by camera 
         at_try(cam, AT_GetFloat, L"ExposureTime", &exp);
     
//       Get constants for this camera
         cam_param( cam );
         cam->Dimension[IMG_WIDTH]  = (int)cam->SensorWidth/img_bin;
         cam->Dimension[IMG_HEIGHT] = (int)cam->SensorHeight/img_bin;
     
//       Exposure info
         cam->ExpReq = exp;                       // Requested exposure time
         cam->ExpDif = cam->ExpReq - cam->ExpVal; // Difference from actual

//       Set image size 
         img_mono16size = (cam->SensorWidth/img_bin) * (cam->SensorHeight/img_bin);  
     
//       Blank space added to line up camera info output 
         mop_log( true, LOG_INF, FAC, 
                           "Ser. No.   = %ls"
                 LOG_BLANK "Model      = %ls"
                 LOG_BLANK "Firmware   = %ls"
                 LOG_BLANK "Trig. Mode = %ls"
                 LOG_BLANK "Encoding   = %ls"
                 LOG_BLANK "Read Rate  = %ls"
                 LOG_BLANK "Amp. Mode  = %ls"
                 LOG_BLANK "Gain       = %f e/ADU"
                 LOG_BLANK "Well Depth = %i e"
                 LOG_BLANK "Dark Curr. = %f e/px/s"
                 LOG_BLANK "Binning    = %ls"
                 LOG_BLANK "FullAOICtl = %s",
                 cam->SerialNumber, cam->Model, cam->FirmwareVersion, 
                 cam_trg, cam_enc, cam_mhz, cam_amp, 
                 cam->Gain, cam->WellDepth, cam->DarkCurrent, cam_bin, btoa(cam->FullAOIControl) );
	 return mop_log( true, LOG_INF, FAC, 
                                   "ImageSize  = 0x%X bytes"
                         LOG_BLANK "ReadoutTime= %.4f s" 
                         LOG_BLANK "ExpTime    = %.4f s" 
                         LOG_BLANK "ExpMax     = %.4f s" 
                         LOG_BLANK "ExpMin     = %.5f s"
                         LOG_BLANK "Exp. / rot.= %i"
                         LOG_BLANK "Exp. Total = %i",
	                 cam->ImageSizeBytes, cam->ReadoutTime, cam->ExpVal, cam->ExpMax, cam->ExpMin, img_cycle, img_total ); 
    }
    return mop_log( false, LOG_ERR, FAC, "cam_conf()" ); 
}


/** @brief     Populate camera data-sheet parameters based on serial number 
  *
  * @param[in] *cam = pointer to camera info structure
  *
  * @return    true | false = Success | Failure
  */
bool cam_param( mop_cam_t *cam )
{
   int c; // Index by camera
   int m; // Index by MHz to get gain

   for ( c = 0; c < CAM_COUNT; c++ )
       if ( !wcscmp( cam_info[c].SerialNumber, cam->SerialNumber ))
       {
//         Easy stuff from look-up table first
           cam->Model       = cam_info[c].Model;
           cam->WellDepth   = cam_info[c].WellDepth;
           cam->DarkCurrent = cam_info[c].DarkCurrent;
           cam->Filter      = cam_info[c].Filter;
           cam->FilterID    = cam_info[c].FilterID;

//         What's the frequency Kenneth? 
           if      (!wcscmp( CAM_MHZ_100, cam_mhz ))
               m = IDX_MHZ_100;
           else if (!wcscmp( CAM_MHZ_270, cam_mhz ))
               m = IDX_MHZ_270;
           else
               return mop_log( false, LOG_ERR, FAC, "cam_param(). Unsupported read-rate." ); 

//         Get the gain and noise for the selected read rate
           if      (!wcscmp( CAM_AMP_16L, cam_amp ))
           {
               cam->Gain  = cam_info[c].mhz[m].gain [IDX_AMP_16L];
               cam->Noise = cam_info[c].mhz[m].noise[IDX_AMP_16L];
           }
           else if (!wcscmp( CAM_AMP_12L, cam_amp ))
           {
               cam->Gain  = cam_info[c].mhz[m].gain [IDX_AMP_12L];
               cam->Noise = cam_info[c].mhz[m].noise[IDX_AMP_12L];
           }
           else if (!wcscmp( CAM_AMP_12H, cam_amp ))
           {
               cam->Gain  = cam_info[c].mhz[m].gain [IDX_AMP_12H];
               cam->Noise = cam_info[c].mhz[m].noise[IDX_AMP_12H];
           }
           else 
           {
               return mop_log( false, LOG_ERR, FAC, "cam_param(). Unsupported pre-amp gain" ); 
           }

           return true;
       }

    return mop_log( false, LOG_ERR, FAC, "cam_param(). Unrecognised camera=%ls", cam->SerialNumber ); 
}


/** @brief     Queue exposure buffers 
  *
  * @param[in] *cam = pointer to camera info structure
  *
  * @return    true | false = Success | Failure
  */
bool cam_queue( mop_cam_t *cam )
{
    int b = 0; // Buffer number

    for ( int i = 0; i < img_total; i++ )
    { 
        if ( !at_chk( AT_QueueBuffer( cam->Handle, cam->ImageBuffer[b], cam->ImageSizeBytes), "QueueBuffer", L""))
            return mop_log( false, LOG_ERR, FAC, "cam_queue()" );

        if ( ++b >= IMG_CYCLE )
             b = 0;       
    }
    return true;
}


/** @brief      Allocate memory blocks for image storage
  *
  * @param[in] *cam = pointer to camera info structure
  *
  * @return     true | false = Success | Failure
  */
bool cam_alloc( mop_cam_t *cam )
{
//  Set maximum size in words for 1x1 binning. Actual image size handled elsewhere 
    img_mono16size =  cam->SensorWidth * cam->SensorHeight;

//  Output will be 16-bit mono so double mono buffer size   
    if ( !( img_mono16 = aligned_alloc( 16,  2 * img_mono16size )))
        return mop_log( false, LOG_SYS, FAC, "Mono16 image aligned_alloc()" );

    for ( int i = 0; i < IMG_CYCLE; i++ )
        if ( !(cam->ImageBuffer[i] = aligned_alloc( 16, 2 * img_mono16size )))
           return mop_log( false, LOG_SYS, FAC, "aligned_alloc(CIRC)" );

    return true;
}


/** @brief     Image acquisition using circular frame buffer
  *
  * @param[in] *cam = pointer to camera info structure
  *
  * @return    true | false = Success | Failure
  */
bool cam_acq_circ( mop_cam_t *cam )
{
    int    b = 0;              // Image buffer
    double rot_req = rot_zero; // Requested rotator angle
    double clk_dif;            // Camera timestamp clock difference
    double rot_now;
    double timeout = TIM_MILLISECOND * cam->ExpVal + TMO_XFR;

    char  msg_buf[1024];
    int   msg_len;
    char *name;
    int   next = FTS_NEXT; 

//  If bias frame then use minimum exposure else restore global value
    if ( fts_pfx == FTS_PFX_BIAS )
    {
        cam->ExpVal = cam->ExpMin;
        at_try(cam, AT_SetFloat, L"ExposureTime", cam->ExpMin );
    }
    else
    {
        cam->ExpVal = cam_exp; 
        at_try(cam, AT_SetFloat, L"ExposureTime", cam_exp );
    }

//  Loop to acquire all images
    for ( int i = 0; i < img_total; i++ )
    {
        gettimeofday(&cam->ObsStart, NULL);

        cam->RotReq[i] = rot_req;
        cam->RotAng[i] = fmod( rot_req, 360.0 );
        cam->RotN[i]   = 1 + (i / img_cycle);
        cam->SeqN[i]   = 1 + (i % img_cycle);

//      Wait for image buffer to be filled.
        if ( !at_chk( AT_WaitBuffer( cam->Handle, &cam->ReturnBuffer[b], &cam->ReturnSize[i], timeout ),"WaitBuffer",L""))
            mop_exit( mop_log( false, LOG_ERR, FAC, "Missed image %i. Exiting", i+1 ));

        if ( mop_master )
        {
//          Get final angle and length of arc for this exposure
            rot_get( &rot_now );
            cam->RotDif[i] = rot_now - rot_req;
            cam->RotEnd[i] = fmod( rot_now, 360.0 );
        }
        else // Slave does not have access to rotator position
        {
//          Fake final angle and length of arc
            cam->RotDif[i] = rot_stp;
            cam->RotEnd[i] = fmod(rot_req + rot_stp, 360.0 );
        }

        cam->TimestampClock[i] = cam_ticks( cam, b );
        if (i)
            clk_dif = (double)(cam->TimestampClock[i] - cam->TimestampClock[i-1]) / cam->TimestampClockFrequency;
        else
            clk_dif = (double)cam->TimestampClock[i] / cam->TimestampClockFrequency;

//      Write to file 
        gettimeofday(&cam->ObsEnd, NULL);
        fts_write( name = fts_mkname( cam, fts_pfx, &next ), cam, i, b );
        msg_len = sprintf( msg_buf, "%s", name );
        msg_send( 0, msg_buf, ipcommand, NULL, 0 ); 

//      Logging
        mop_log( true, LOG_IMG, FAC,
                "Exp %2.2i %-2.2i %f Rot %9.2f %7.2f %7.2f Dif %6.2f %6.4f %5.4f %c",
                 cam->RotN[i],   cam->SeqN[i],   cam->ExpVal, cam->RotReq[i], cam->RotAng[i],
                 cam->RotEnd[i], cam->RotDif[i], fabs(cam->RotDif[i]/rot_vel - rot_sign*cam->ExpVal), clk_dif,
                 i+1 == img_total ? '#':' ' ); // Mark last image 

        rot_req += rot_stp;
        if ( ++b >= MAX_CYCLE )
            b = 0; // Loop circular buffer back to start 
    }

//  Stop acquisition, get temperature and don't forget to flush
    cam_acq_ena( cam, AT_FALSE   );
    cam_trg_set( cam, CAM_TRG_SW );
    at_try( cam, AT_GetFloat,  L"SensorTemperature", &cam->SensorTemperature );
    at_try( cam, AT_Flush   ,  L"", NULL );

    return true;
}

/** @brief      Image acquisition loop (static)  
  *
  * @param[in] *cam = pointer to camera info structure
  *
  * @return     true | false = Success | Failure
  */
bool cam_acq_stat( mop_cam_t *cam )
{
    int    b = 0;              // Image buffer
    double rot_req = rot_zero; // Requested rotator angle. Default zero position
    double clk_dif;            // Camera timestamp clock difference
    double timeout = TIM_MILLISECOND * cam->ExpVal + TMO_XFR;

    char msg_buf[1024];
    int  msg_len;

    char *name;
    int   next = FTS_NEXT;

//  If bias frame then use minimum exposure else restore global value
    if ( fts_pfx == FTS_PFX_BIAS )
    {
        cam->ExpVal = cam->ExpMin;
        at_try(cam, AT_SetFloat, L"ExposureTime", cam->ExpMin );
    }
    else
    {
        cam->ExpVal = cam_exp; 
        at_try(cam, AT_SetFloat, L"ExposureTime", cam_exp );
    }

//  Loop to acquire images
    for ( int i = 0; i < img_total; i++ )
    {
        gettimeofday(&cam->ObsStart, NULL);

        cam->RotReq[i] = rot_req;                 // Absolute rotation
        cam->RotAng[i] = fmod( rot_req, 360.0 );  // 0-360 rotation
        cam->RotN[i]   = 1 + (i / img_cycle);     // Rotation number
        cam->SeqN[i]   = 1 + (i % img_cycle);     // Position within rotation

        if ( mop_master ) // Master process
        {
            if ( !rot_goto( rot_req, TMO_ROTATOR, &cam->RotEnd[i] ) )
                return mop_log( false, LOG_ERR, FAC, "rot_goto(Static)");
           
//          If not single camera mode send signal to slave 
            if ( !one_cam )
            {
                if ( msg_send( TMO_MSG, MSG_TRG, ipslave, MSG_ACK, strlen(MSG_ACK) ) )
                    mop_log( true, LOG_DBG, FAC, "Master sent SW trigger" );
                else
                    return mop_log( false, LOG_ERR, FAC, "msg_send()");
            }
        }
        else // Slave process 
        {
            cam->RotEnd[i] = rot_req;
            if ( !msg_recv( TMO_MSG, msg_buf, sizeof(msg_buf), &msg_len, MSG_TRG, strlen(MSG_TRG)))
                return mop_log( false, LOG_ERR, FAC, "cam_acq_stat() timeout.");
            mop_log( true, LOG_DBG, FAC, "Slave received SW trigger" );
        }

        at_try( cam, AT_Command, L"SoftwareTrigger", NULL );
        if (!at_chk( AT_WaitBuffer(cam->Handle, &cam->ReturnBuffer[b], &cam->ReturnSize[i], timeout),"WaitBuffer",L""))
            return mop_log( false, LOG_ERR, FAC, "Missed image %i", i+1 );

        cam->RotEnd[i] = fmod( cam->RotEnd[i], 360.0 );
        cam->RotDif[i] = cam->RotEnd[i] - cam->RotAng[i];
        cam->TimestampClock[i] = cam_ticks( cam, b );
        if (i)
            clk_dif = (double)(cam->TimestampClock[i] - cam->TimestampClock[i-1]) / cam->TimestampClockFrequency;
        else
            clk_dif = (double)cam->TimestampClock[i] / cam->TimestampClockFrequency;

//      Write to file 
        gettimeofday(&cam->ObsEnd, NULL);
        fts_write( fts_mkname( cam, fts_pfx, &next ), cam, i, b );
        msg_len = sprintf( msg_buf, "%s", name );
        msg_send( 0, msg_buf, ipcommand, NULL, 0 ); 

//      Logging
        mop_log( true, LOG_IMG, FAC,
                "Exp %2.2i %-2.2i %f Rot %9.2f %7.2f %7.2f Dif %6.2f %6.4f %5.4f %c",
                 cam->RotN[i],   cam->SeqN[i],   cam->ExpVal, cam->RotReq[i], cam->RotAng[i],
                 cam->RotEnd[i], cam->RotDif[i], fabs(cam->RotDif[i]/rot_vel - rot_sign*cam->ExpVal), clk_dif,
                 i+1 == img_total ? '#':' ' ); // Mark final image 

        rot_req += rot_stp;
        if ( ++b >= MAX_CYCLE )
            b = 0; // Loop circular buffer back to start 
    }

//  Stop acquisition, get temperature and don't forget to flush   
    at_try( cam, AT_Command,  L"AcquisitionStop", NULL);
    at_try( cam, AT_GetFloat, L"SensorTemperature", &cam->SensorTemperature);
    at_try( cam, AT_Flush,    L"", NULL);

    return true;
}


/** @brief      Read camera ticks from image meta-data.
  *             Meta-data reporting must be enabled.  
  *
  * @param[in] *cam = pointer to camera info structure
  * @param[in]  buf = Index into image buffer array 
  *
  * @return     Image size or 0 = Failure  
  */
unsigned long cam_ticks( mop_cam_t *cam, int buf )
{
    int i;

    unsigned char *ptr1;
    unsigned int len;
    unsigned int cid;

    ptr1 = cam->ImageBuffer[buf] + cam->ImageSizeBytes;

// DEBUG:  Used for per-byte verify of meta-data structure    
//    for( i = 0; i < 40; i++ )
//      printf( "%i=%i\n", i, *--ptr0 );

//  Handles Andor's tacked-on meta-data, see Sec 4.5 METADATA in SDK 3.11 for gruesome details
//  Parse backwards from the end of image 
    for( i = 0; i < 3; i++ )
    {
//      Move back and read int length field
        ptr1 -= 4;
        len = *(unsigned int *)ptr1;

//      Move back and read int CID field        
        ptr1 -= 4;
        cid = *(unsigned int *)ptr1;

//      Move back and point to data by subtracting CID field length     
        ptr1 -= (len - 4);
        switch ( cid )
        {
            case 0:
// DEBUG         printf( "cid=%i len=%i\n", cid, len-4 );
                 break;
            case 1:
// DEBUG         printf( "cid=%i ticks=%lu\n", cid, *(unsigned long*)ptr1 );
                 return *(unsigned long*)ptr1;
                 break;
            case 7:
// DEBUG         printf( "cid=%i\n", cid );
                 break;

            default:
                 break;
        }
    }

    return 0;
}


/** @brief      Debug: Show implementation state of an enumerated camera feature 
  *
  * @param[in] *cam     = pointer to camera info structure 
  * @param[in]  Feature = Enumerate feature name 
  * 
  * @return     void
  */
void cam_feature( mop_cam_t *cam, AT_WC *Feature )
{	
    int i;
    int Count;
    AT_WC String[MAX_STR];
    AT_BOOL Available;
    AT_BOOL Implemented;
    
    at_chk( AT_GetEnumCount( cam->Handle, Feature, &Count ),"GetEnumCount()",L"");

//  Print out what is available and implemented    
    puts( "Avail. Impl." );
    for ( i = 0; i < Count; i++ )
    {
        at_chk(AT_GetEnumStringByIndex  (cam->Handle,Feature,i,String, MAX_STR-1),"GetEnumStringByIndex",L"");
        at_chk(AT_IsEnumIndexAvailable  (cam->Handle,Feature,i,&Available  ),"IsEnumIndexAvailable"  ,L"");
        at_chk(AT_IsEnumIndexImplemented(cam->Handle,Feature,i,&Implemented),"IsEnumIndexImplemented",L"");
        printf( "%-5s  %-5s  %ls\n", btoa(Available), btoa(Implemented), String );
    }

    at_chk(AT_GetEnumIndex(cam->Handle, Feature, &i ),"SetEnumIndex",L"");
    at_chk(AT_GetEnumStringByIndex(cam->Handle,Feature,i,String,MAX_STR-1),"GetEnumStringByIndex",L"");
    printf( "\nActual = %ls\n", String );
}


/** @brief      Enable or disable camera acquisition      
  *
  * @param[in] *cam = pointer to camera info structure 
  * @param[in]  new = New state AT_TRUE (enable) or AT_FALSE (disable)
  * 
  * @return     true | false = Success | Failure 
  */
bool cam_acq_ena( mop_cam_t *cam, AT_BOOL new )
{
    AT_BOOL now;

    at_try( cam, AT_GetBool, L"CameraAcquiring", &now ); 
    if ( now != new )
    {
        if ( new == AT_TRUE ) 
        {
           if (at_try( cam, AT_Command, L"AcquisitionStart", NULL))
               return mop_log( true , LOG_INF, FAC, "Command(AcquisitionStart)");
           else
               return mop_log( false, LOG_ERR, FAC, "Command(AcquisitionStart)");
        }
        else 
        {
            if (at_try( cam, AT_Command, L"AcquisitionStop", NULL))
                return mop_log( true , LOG_INF, FAC, "Command(AcquisitionStop)");
            else
                return mop_log( false, LOG_ERR, FAC, "Command(AcquisitionStop)");
        }
    }

    return true;
}


/** @brief     Set camera trigger mode      
  *
  * @param[in] *cam = pointer to camera info structure 
  * @param[in] *trg = pointer to trigger mode string (unicode) 
  * 
  * @return    true | false - Success | Failure 
  */
bool cam_trg_set( mop_cam_t *cam, AT_WC *trg )
{
   return at_try( cam, AT_SetEnumString, L"TriggerMode", trg );
}


/** @brief      Reset camera internal clock        
  *
  * @param[in] *cam = pointer to camera info structure 
  * 
  * @return     true | false - Success | Failure 
  */
bool cam_clk_rst( mop_cam_t *cam )
{
   return at_try( cam, AT_Command, L"TimestampClockReset", NULL); 	
}
