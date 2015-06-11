/*************************************************************************

    SGMOOS: A set of libraries, scripts & applications which extend MOOS 
    (Mission Oriented Operating Suite by Dr. Paul Newman) for use in 
    Autonomous Underwater Vehicles & Autonomous Surface Vessels. 

    Copyright (C) 2006,2007,2008,2009 Massachusetts Institute of 
    Technology Sea Grant

    This software was written by Justin Eskesen & others at MIT Sea Grant.
    contact: jge@mit.edu

    This file is part of SGMOOS.

    SGMOOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SGMOOS.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************/
// CameraDriverDC1394.cpp: implementation of the CCameraDriverDC1394 class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _WIN32
	#pragma warning(disable : 4786)
	#pragma warning(disable : 4503)
#endif

#include "CameraDriverDC1394.h"
#include <string.h>

#define ERR_1394   "Unable to acquire a raw1394 handle (check rw access to /dev/raw1394)"
#define ERR_NOCAM  "No cameras found"
#define ERR_ROOT   "Camera has become root node of 1394 bus"
#define ERR_ISO    "Unable to get iso channel number"
#define ERR_SETUP  "Unable to setup camera (check framerate and format)"
#define ERR_TRANS  "Unable to start camera iso transmission"
#define ERR_FRAME  "Unable to capture a frame"
#define ERR_ALLOC  "Unable to allocate buffer"
#define ERR_FILE   "Unable to create file"
#define ERR_STOP   "Unable to stop camera (uhh, what?)"


using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCameraDriverDC1394::CCameraDriverDC1394()
{
    
    MOOSTrace("WONDERFUL THINGS ARE HAPPENING ALL AROUND US\n");    


}


CCameraDriverDC1394::~CCameraDriverDC1394()
{
    //Cleanup(); called by driver class
}

bool CCameraDriverDC1394::CleanupHelper()
{
    MOOSTrace("DC1394 driver cleaning up\n");

    //initialize all variables to nothing
    m_iMode           = -1;
    m_iFormat         = -1;
    m_iFramerate      = -1;
    m_iTriggermode    = -1;
    m_iDataspeed      = -1;
    m_iEncoding       = -1;
    m_iBytesPerPixel  = -1;
    m_i1394Card       = -1;
    m_iCameraNode     = -1;

    m_sVideoDevice.clear();

    //tear down camera if created
    if (m_bDc1394CameraCreated)
    {
        MOOSTrace("Unlistening DMA on 1394 handle\n");
        dc1394_dma_unlisten(m_raw1394Handle, &m_dc1394Camera);
        MOOSTrace("Releasing camera\n");
        dc1394_dma_release_camera(m_raw1394Handle, &m_dc1394Camera);
    }
    
    if ( NULL != m_raw1394Handle )
    {
        MOOSTrace("Releasing handle\n");
        raw1394_destroy_handle(m_raw1394Handle);
    }

    return m_raw1394Handle == NULL;
    
}

bool CCameraDriverDC1394::GotAllConfigs()
{
    return (
        m_iMode           != -1 &&
        m_iFormat         != -1 &&
        m_iFramerate      != -1 &&
        //m_iTriggermode    != -1 && //optional!  not sure where i even use this
        m_iDataspeed      != -1 &&
        m_iEncoding       != -1 &&
        m_iBytesPerPixel  != -1 &&
        m_i1394Card       != -1 &&
        m_iCameraNode     != -1 &&

        !m_sVideoDevice.empty() &&
        true //this line is here so that all previous can end with &&

        );
}

//uses a work function to lookup the enum val of the string given, returns the value
int CCameraDriverDC1394::getCfgEnumValue(const char* s, int (*work)(const char*), bool reqd)
{
    std::string tmp;
    int i;
    
    if (!m_support->ConfigRead(s, tmp))
    {
        if (!reqd)
        {
            MOOSTrace("Optional ");
        }
        MOOSTrace("Configuration variable not found: %s\n", s);
    }

    i = (work)(tmp.c_str());

    MOOSTrace("Configuration variable %s:%s = %i\n", s, tmp.c_str(), i);

    return i;
}

//read what we need from config file
bool CCameraDriverDC1394::GetConfigs()
{
    std::string tmp;

    m_iMode           = getCfgEnumValue("MODE", &str2mode, true);
    m_iFormat         = getCfgEnumValue("FORMAT", &str2format, true);
    m_iFramerate      = getCfgEnumValue("FRAMERATE", &str2framerate, true);
    m_iTriggermode    = getCfgEnumValue("TRIGGERMODE", &str2triggermode, false);
    m_iDataspeed      = getCfgEnumValue("DATASPEED", &str2dataspeed, true);
    m_iEncoding       = getCfgEnumValue("ENCODING", &str2encoding, true);

    //get bytes per pixel
    if (!m_support->ConfigRead("BYTESPERPIXEL", tmp))
    {
        MOOSTrace("Configuration variable BYTESPERPIXEL not found!\n");
    }
    m_iBytesPerPixel = atoi(tmp.c_str());


    //get 1394 card 
    if (!m_support->ConfigRead("1394_CARD", tmp))
    {
        MOOSTrace("Configuration variable 1394_CARD not found!\n");
    }
    m_i1394Card  = atoi(tmp.c_str());

    //get camera node (on 1394 bus)
    if (!m_support->ConfigRead("CAMERA_NODE", tmp))
    {
        MOOSTrace("Configuration variable CAMERA_NODE not found!\n");
    }
    m_iCameraNode   = atoi(tmp.c_str());

    //this is the /dev/videoWhatever 
    if (!m_support->ConfigRead("DEVICE", m_sVideoDevice))
    {
        MOOSTrace("Configuration variable DEVICE not found!\n");
    }

    return GotAllConfigs();

}


bool CCameraDriverDC1394::SetupHelper()
{
    //init vars, dispose of any previous stuff
    CleanupHelper();

    //bail out if not all conifgs are supplied
    if (GetConfigs())
    {
        MOOSTrace("All necessary config vars have been supplied :)\n");
    }
    else
    {
        MOOSTrace("Some config vars missing, setup is aborting\n");
        return false;
    }
    
    //open ohci and assign handle
    m_raw1394Handle = dc1394_create_handle(m_i1394Card);
    if ( m_raw1394Handle == NULL )
    {
        this->ReportError(ERR_1394 "\n");
        MOOSTrace(ERR_1394);
        return false;
    }


    // get the camera nodes and describe them as we find them
    int numNodes;
    numNodes = raw1394_get_nodecount(m_raw1394Handle);

    nodeid_t *   cameraNodes;
    int          numCameras;
    /* Note: set 3rd parameter to 0 if you do not want the camera details printed */
    cameraNodes = dc1394_get_camera_nodes(m_raw1394Handle, &numCameras, 1);

    //need at least one camera
    if (numCameras < 1)
    {
        this->ReportError(ERR_NOCAM "\n");
        MOOSTrace(ERR_NOCAM);        
        return false;
    }

    //make sure camera has not made itself the root node
    if( cameraNodes[0] == numNodes-1)
    {
        this->ReportError(ERR_ROOT "\n");
        MOOSTrace(ERR_ROOT);                
        return false;
    }

    //of all the 1394 devices, choose the proper node
    m_cameraNode = cameraNodes[m_iCameraNode];


    //setup capture
    unsigned int channel;
    unsigned int speed;
    if (DC1394_SUCCESS != dc1394_get_iso_channel_and_speed(m_raw1394Handle,
                                                           m_cameraNode,
                                                           &channel,
                                                           &speed))
    {
        this->ReportError(ERR_ISO "\n");
        MOOSTrace(ERR_ISO);                
        return false;
    }


    /* Setup the capture mode */
    int e = dc1394_dma_setup_capture( m_raw1394Handle,
                                      m_cameraNode,
                                      channel,
                                      m_iFormat,
                                      m_iMode,
                                      m_iDataspeed,
                                      m_iFramerate,
                                      8, // number of buffers
                                      1, // drop frames
                                      m_sVideoDevice.c_str(),
                                      &m_dc1394Camera );
    if (e != DC1394_SUCCESS)
    {
        this->ReportError(ERR_SETUP "\n");
        MOOSTrace(ERR_SETUP);
        return false;
    }
    m_bDc1394CameraCreated = true;

    //create MOOS registers for various settings
    if (m_bDc1394CameraCreated)
    {
        CreateCameraRegisters();
    }

    return m_bDc1394CameraCreated;
}

void CCameraDriverDC1394::CreateCameraRegisters()
{
    CreateRegisterNamed("Brightness",       &dc1394_get_brightness);
    CreateRegisterNamed("Exposure",         &dc1394_get_exposure);
    CreateRegisterNamed("Sharpness",        &dc1394_get_sharpness);
    CreateRegisterNamed("Hue",              &dc1394_get_hue);
    CreateRegisterNamed("Saturation",       &dc1394_get_saturation);
    CreateRegisterNamed("Gamma",            &dc1394_get_gamma);
    CreateRegisterNamed("Shutter",          &dc1394_get_shutter);
    CreateRegisterNamed("Gain",             &dc1394_get_gain);
    CreateRegisterNamed("Iris",             &dc1394_get_iris);
    CreateRegisterNamed("Focus",            &dc1394_get_focus);
    //CreateRegisterNamed("Temperature",      &dc1394_get_temperature);
    //CreateRegisterNamed("White Shading",    &dc1394_get_white_shading);
    CreateRegisterNamed("Trigger Delay",    &dc1394_get_trigger_delay);
    CreateRegisterNamed("Frame Rate",       &dc1394_get_frame_rate);
    CreateRegisterNamed("Trigger Mode",     &dc1394_get_trigger_mode);
    CreateRegisterNamed("Zoom",             &dc1394_get_zoom);
    CreateRegisterNamed("Pan",              &dc1394_get_pan);
    CreateRegisterNamed("Tilt",             &dc1394_get_tilt);
    CreateRegisterNamed("Optical Filter",   &dc1394_get_optical_filter);
    CreateRegisterNamed("Capture Size",     &dc1394_get_capture_size);
    CreateRegisterNamed("Capture Quality",  &dc1394_get_capture_quality);
}

void CCameraDriverDC1394::GeneralHandleUInt(const char* suffix, 
                                            MOOSMSG_LIST &NewMail, 
                                            int (*work)(raw1394handle_t, nodeid_t, unsigned int))
{
    CMOOSMsg Msg;
    std::string s(GetAppName());

    s += "_";
    s += suffix;

    //convert to uppercase
    std::transform(s.begin(), s.end(), s.begin(), (int(*)(int)) ::toupper);

    if(!m_support->CommsPeekMail(NewMail, s, Msg))
    {
        //MOOSTrace("  %s", s.c_str());
    }
    else
    {
        MOOSTrace(" Got %s...", s.c_str());
        if(Msg.IsSkewed(MOOSTime()))
        {
            MOOSTrace("Skewed\n");
        }
        else
        {
            MOOSTrace("%f\n", Msg.GetDouble());
            GeneralUintSet(suffix, work, Msg.GetDouble());
        }
    }
}

bool CCameraDriverDC1394::HandleMail(MOOSMSG_LIST &NewMail)
{
    MOOSTrace("Got new mail in DC1394!\n");
    GeneralHandleUInt("Brightness",       NewMail, &dc1394_set_brightness);
    GeneralHandleUInt("Exposure",         NewMail, &dc1394_set_exposure);
    GeneralHandleUInt("Sharpness",        NewMail, &dc1394_set_sharpness);
    GeneralHandleUInt("Hue",              NewMail, &dc1394_set_hue);
    GeneralHandleUInt("Saturation",       NewMail, &dc1394_set_saturation);
    GeneralHandleUInt("Gamma",            NewMail, &dc1394_set_gamma);
    GeneralHandleUInt("Shutter",          NewMail, &dc1394_set_shutter);
    GeneralHandleUInt("Gain",             NewMail, &dc1394_set_gain);
    GeneralHandleUInt("Iris",             NewMail, &dc1394_set_iris);
    GeneralHandleUInt("Focus",            NewMail, &dc1394_set_focus);
    //GeneralHandleUInt("Temperature",      NewMail, &dc1394_set_temperature);
    //GeneralHandleUInt("White Shading",    NewMail, &dc1394_set_white_shading);
    GeneralHandleUInt("Trigger Delay",    NewMail, &dc1394_set_trigger_delay);
    GeneralHandleUInt("Frame Rate",       NewMail, &dc1394_set_frame_rate);
    GeneralHandleUInt("Trigger Mode",     NewMail, &dc1394_set_trigger_mode);
    GeneralHandleUInt("Zoom",             NewMail, &dc1394_set_zoom);
    GeneralHandleUInt("Pan",              NewMail, &dc1394_set_pan);
    GeneralHandleUInt("Tilt",             NewMail, &dc1394_set_tilt);
    GeneralHandleUInt("Optical Filter",   NewMail, &dc1394_set_optical_filter);
    GeneralHandleUInt("Capture Size",     NewMail, &dc1394_set_capture_size);
    GeneralHandleUInt("Capture Quality",  NewMail, &dc1394_set_capture_quality);

    return true;
}

bool CCameraDriverDC1394::GrabHelper(std::string sDirectory, std::string sFileprefix)
{

    if (!GrabImg())
    {
        //FAIL... but we return true? FIXME, what do i want to do here
        return true;
    }


    char pDirPath[1024];

    sprintf(pDirPath, "%s/%s", 
            sDirectory.c_str(),
            sFileprefix.c_str());

    
    return Save(pDirPath);
    
}

bool CCameraDriverDC1394::GrabImg()
{

    if (!m_bDc1394CameraCreated)
    {
        MOOSTrace("Tried to grab an image without camera being set up!");
        return true;
    }

    //call base class
    //CCameraDriver::Grab();

    // have the camera start sending us data
    if (DC1394_SUCCESS != dc1394_start_iso_transmission( m_raw1394Handle, m_dc1394Camera.node))
    {
        this->ReportError(ERR_TRANS "\n");
        MOOSTrace(ERR_TRANS);                        
        return true;
    }

    // capture one frame to save to file
    if (DC1394_SUCCESS != dc1394_dma_single_capture( &m_dc1394Camera))
    {
        this->ReportError(ERR_FRAME "\n");
        MOOSTrace(ERR_FRAME);                        
        return true;
    }

    // pick up image dimensions while they are hot
    m_iFrameWidth = m_dc1394Camera.frame_width;
    m_iFrameHeight = m_dc1394Camera.frame_height;

    int bufferSize = m_iFrameWidth * m_iFrameHeight;

    if (m_rgbBuffer)
    {
        delete m_rgbBuffer;
    }
    
    //create destination
    m_rgbBuffer = new unsigned char[3 * bufferSize];
    if ( !m_rgbBuffer )
    {
        this->ReportError(ERR_ALLOC " for color conversion");
        MOOSTrace(ERR_ALLOC " for color conversion");

        //not check for errors... already trouble, so not sure what to do
        dc1394_stop_iso_transmission(m_raw1394Handle, m_dc1394Camera.node);
        dc1394_dma_done_with_buffer( &m_dc1394Camera );

        return  true;
    }

        
    unsigned char* src = (unsigned char*) m_dc1394Camera.capture_buffer;
    if ( m_iBytesPerPixel > 1 )
    {
        src = new unsigned char[ bufferSize ];
        if ( !src )
        {
            
            this->ReportError(ERR_ALLOC " for capture");
            MOOSTrace(ERR_ALLOC " for capture");
            
            //not checking for errors... already trouble, so not sure what to do
            dc1394_stop_iso_transmission(m_raw1394Handle, m_dc1394Camera.node);
            dc1394_dma_done_with_buffer( &m_dc1394Camera );
            
            return  true;
            
        }
        unsigned char* captureBuffer = (unsigned char*) m_dc1394Camera.capture_buffer;
        for ( int i = 0; i < bufferSize; i++ )
        {
            src[i] = captureBuffer[ i * m_iBytesPerPixel ];
        }
    }
    
    /*-----------------------------------------------------------------------
     *  convert to color image
     *-----------------------------------------------------------------------*/
    
    switch (m_iEncoding)
    {
    case ENCODING_RGB:
        break; //done!
        
    case ENCODING_RGB48:
        rgb482rgb(src, m_rgbBuffer, (m_iFrameWidth * m_iFrameHeight));
        break;
        
    case ENCODING_UYV:
        uyv2rgb(src, m_rgbBuffer, (m_iFrameWidth * m_iFrameHeight));
        break;
        
    case ENCODING_UYVY:
        uyvy2rgb(src, m_rgbBuffer, (m_iFrameWidth * m_iFrameHeight));
        break;
        
    case ENCODING_UYYVYY:
        uyyvyy2rgb(src, m_rgbBuffer, (m_iFrameWidth * m_iFrameHeight));
        break;
        
    case ENCODING_Y:
        y2rgb(src, m_rgbBuffer, (m_iFrameWidth * m_iFrameHeight));
        break;
        
    case ENCODING_Y16:
        y162rgb(src, m_rgbBuffer, (m_iFrameWidth * m_iFrameHeight), m_iBytesPerPixel);
        break;
        
   case ENCODING_BAYER:
       
       BayerNearestNeighbor( src, 
                             m_rgbBuffer, 
                             m_iFrameWidth,
                             m_iFrameHeight,
                             (bayer_pattern_t) GetBayerPattern() );
       break;
       
    default:
        MOOSTrace("Unknown encoding specified!");
    }
    
    
    //stop data transmission
    if (DC1394_SUCCESS != dc1394_stop_iso_transmission(m_raw1394Handle, m_dc1394Camera.node))
    {
        printf("Couldn't stop the camera?\n");
    }
    
    dc1394_dma_done_with_buffer( &m_dc1394Camera );

    
    return true;
}


bool CCameraDriverDC1394::Save(std::string sFilename)
{
    FILE* imagefile;

    char pDirPath[1024];

    sprintf(pDirPath, "%s.ppm", 
            sFilename.c_str());


    /*-----------------------------------------------------------------------
     *  save image 
     *-----------------------------------------------------------------------*/
    MOOSTrace( "Saving %s...", pDirPath);
    imagefile = fopen( pDirPath, "w" );

    if( imagefile == NULL)
    {
        this->ReportError("Can't create " + sFilename);
        MOOSTrace("Can't create '%s'\n", pDirPath);
    }

    fprintf( imagefile,
             "P6\n%u %u 255\n",
             m_iFrameWidth,
             m_iFrameHeight);

    fwrite(m_rgbBuffer, 
           3,
           m_iFrameWidth * m_iFrameHeight, 
           imagefile);
    
    fclose(imagefile);
    MOOSTrace("done saving\n", pDirPath);

    //notify with the filename
    this->ReportSavedImage(sFilename + ".ppm");

    return true;
}


void CCameraDriverDC1394::dumpCameraInfo()
{
    CheckReadiness();
        
    /* Get the camera information and print that structure*/
    dc1394_camerainfo info;
    
    dc1394_get_camera_info(m_raw1394Handle, m_cameraNode, &info);
    
    
    dc1394_print_camera_info(&info);
    
    /*
    // Functions for querying camera attributes 
    int
        dc1394_query_supported_formats(m_raw1394Handle, m_cameraNode,
                                       quadlet_t *value);
    int
        dc1394_query_supported_modes(m_raw1394Handle, m_cameraNode,
                                     unsigned int format, quadlet_t *value);
    int
        dc1394_query_supported_framerates(m_raw1394Handle, m_cameraNode,
                                          unsigned int format, unsigned int mode,
                                          quadlet_t *value);
    int
        dc1394_query_revision(m_raw1394Handle, m_cameraNode, int mode,
                              quadlet_t *value);
    int
        dc1394_query_basic_functionality(m_raw1394Handle, m_cameraNode,
                                         quadlet_t *value);
    
    
   // Get/Set the framerate, mode, format, iso channel/speed for the video 
    int
        dc1394_get_video_framerate(m_raw1394Handle, m_cameraNode,
                                   unsigned int *framerate);
    int
        dc1394_get_video_mode(m_raw1394Handle, m_cameraNode,
                              unsigned int *mode);
    int
        dc1394_get_video_format(m_raw1394Handle, m_cameraNode,
                                unsigned int *format);
    int
        dc1394_get_operation_mode(m_raw1394Handle, m_cameraNode,
                              unsigned int *mode);
    int
        dc1394_get_iso_channel_and_speed(m_raw1394Handle, m_cameraNode,
                                         unsigned int *channel, unsigned int *speed);

    */
}



void CCameraDriverDC1394::CheckReadiness()
{
    if (!m_bDc1394CameraCreated)
    {
        MOOSTrace("Tried to do something camera-related without camera being set up!");
    }
}

void CCameraDriverDC1394::Check1394Success(int r, const char* feature)
{
    if (DC1394_SUCCESS != r)
    {
        std::string f(feature);
        //{ FIXME: call error function, use feature name }
        this->ReportError("Feature not supported: " + f);
    }

}


//this function creates a MOOS register if the feature is supported
void CCameraDriverDC1394::CreateRegisterNamed(const char* feature, 
                            int (*work)(raw1394handle_t, nodeid_t, unsigned int*))
{
    int r;
    unsigned int v;
    std::string s(GetAppName());

    s += "_";
    s += feature;

    //convert to uppercase
    std::transform(s.begin(), s.end(), s.begin(), (int(*)(int)) ::toupper);

    CheckReadiness();

    r = (*work)(m_raw1394Handle, m_cameraNode, &v);

    if (DC1394_SUCCESS == r)
    {
        m_support->CommsNotify(s, v);
        m_support->CommsRegister(s);
    }
}

//this function makes my life easier by doing all the work associated with a get uint
//... pre-check, error check, and retval
unsigned int CCameraDriverDC1394::GeneralUintGet(const char* feature, 
                            int (*work)(raw1394handle_t, nodeid_t, unsigned int*))
{
    int r;
    unsigned int v;

    CheckReadiness();

    r = (*work)(m_raw1394Handle, m_cameraNode, &v);

    Check1394Success(r, feature);

    return v;
}

//this function makes my life easier by doing all the work associated with a set uint
//... pre-check, error check, and retval
void CCameraDriverDC1394::GeneralUintSet(const char* feature, 
                                         int (*work)(raw1394handle_t, nodeid_t, unsigned int),
                                         unsigned int v)
{
    int r;

    CheckReadiness();

    r = (*work)(m_raw1394Handle, m_cameraNode, v);

    Check1394Success(r, feature);
}


/* Get/Set the values of the various features on the camera */

//this is the only weird one
void CCameraDriverDC1394::GetWhteBalance(unsigned int* blueval, unsigned int* redval)
{
    int r;

    CheckReadiness();

    r = dc1394_get_white_balance(m_raw1394Handle, m_cameraNode, blueval, redval);

    Check1394Success(r, "White Balance");
}

void CCameraDriverDC1394::SetWhiteBalance(unsigned int blueval, unsigned int redval)
{
    int r;

    CheckReadiness();
    
    r = dc1394_set_white_balance(m_raw1394Handle, m_cameraNode, blueval, redval);

    Check1394Success(r, "White Balance");
}


//these are all simple UINT set/get

unsigned int CCameraDriverDC1394::GetBrightness()
{
    return GeneralUintGet("Brightness", &dc1394_get_brightness);
}

void CCameraDriverDC1394::SetBrightness(unsigned int v)
{
    return GeneralUintSet("Brightness", &dc1394_set_brightness, v);
}


unsigned int CCameraDriverDC1394::GetExposure()
{
    return GeneralUintGet("Exposure", &dc1394_get_exposure);
}

void CCameraDriverDC1394::SetExposure(unsigned int v)
{
    return GeneralUintSet("Exposure", &dc1394_set_exposure, v);
}

unsigned int CCameraDriverDC1394::GetSharpness()
{
    return GeneralUintGet("Sharpness", &dc1394_get_sharpness);
}

void CCameraDriverDC1394::SetSharpness(unsigned int v)
{
    return GeneralUintSet("Sharpness", &dc1394_set_sharpness, v);
}

unsigned int CCameraDriverDC1394::GetHue()
{
    return GeneralUintGet("Hue", &dc1394_get_hue);
}

void CCameraDriverDC1394::SetHue(unsigned int v)
{
    return GeneralUintSet("Hue", &dc1394_set_hue, v);
}

unsigned int CCameraDriverDC1394::GetSaturation()
{
    return GeneralUintGet("Saturation", &dc1394_get_saturation);
}

void CCameraDriverDC1394::SetSaturation(unsigned int v)
{
    return GeneralUintSet("Saturation", &dc1394_set_saturation, v);
}

unsigned int CCameraDriverDC1394::GetGamma()
{
    return GeneralUintGet("Gamma", &dc1394_get_gamma);
}

void CCameraDriverDC1394::SetGamma(unsigned int v)
{
    return GeneralUintSet("Gamma", &dc1394_set_gamma, v);
}

unsigned int CCameraDriverDC1394::GetShutter()
{
    return GeneralUintGet("Shutter", &dc1394_get_shutter);
}

void CCameraDriverDC1394::SetShutter(unsigned int v)
{
    return GeneralUintSet("Shutter", &dc1394_set_shutter, v);
}

unsigned int CCameraDriverDC1394::GetGain()
{
    return GeneralUintGet("Gain", &dc1394_get_gain);
}

void CCameraDriverDC1394::SetGain(unsigned int v)
{
    return GeneralUintSet("Gain", &dc1394_set_gain, v);
}

unsigned int CCameraDriverDC1394::GetIris()
{
    return GeneralUintGet("Iris", &dc1394_get_iris);
}

void CCameraDriverDC1394::SetIris(unsigned int v)
{
    return GeneralUintSet("Iris", &dc1394_set_iris, v);
}

unsigned int CCameraDriverDC1394::GetFocus()
{
    return GeneralUintGet("Focus", &dc1394_get_focus);
}

void CCameraDriverDC1394::SetFocus(unsigned int v)
{
    return GeneralUintSet("Focus", &dc1394_set_focus, v);
}

/*
unsigned int CCameraDriverDC1394::GetTemperature()
{
    return GeneralUintGet("Temperature", &dc1394_get_temperature);
}

void CCameraDriverDC1394::SetTemperature(unsigned int v)
{
    return GeneralUintSet("Temperature", &dc1394_set_temperature, v);
}

unsigned int CCameraDriverDC1394::GetWhiteShading()
{
    return GeneralUintGet("White Shading", &dc1394_get_white_shading);
}

void CCameraDriverDC1394::SetWhiteShading(unsigned int v)
{
    return GeneralUintSet("White Shading", &dc1394_set_white_shading, v);
}
*/

unsigned int CCameraDriverDC1394::GetTriggerDelay()
{
    return GeneralUintGet("Trigger Delay", &dc1394_get_trigger_delay);
}

void CCameraDriverDC1394::SetTriggerDelay(unsigned int v)
{
    return GeneralUintSet("Trigger Delay", &dc1394_set_trigger_delay, v);
}

unsigned int CCameraDriverDC1394::GetFrameRate()
{
    return GeneralUintGet("Frame Rate", &dc1394_get_frame_rate);
}

void CCameraDriverDC1394::SetFrameRate(unsigned int v)
{
    return GeneralUintSet("Frame Rate", &dc1394_set_frame_rate, v);
}

unsigned int CCameraDriverDC1394::GetTriggerMode()
{
    return GeneralUintGet("Trigger Mode", &dc1394_get_trigger_mode);
}

void CCameraDriverDC1394::SetTriggerMode(unsigned int v)
{
    return GeneralUintSet("Trigger Mode", &dc1394_set_trigger_mode, v);
}

unsigned int CCameraDriverDC1394::GetZoom()
{
    return GeneralUintGet("Zoom", &dc1394_get_zoom);
}

void CCameraDriverDC1394::SetZoom(unsigned int v)
{
    return GeneralUintSet("Zoom", &dc1394_set_zoom, v);
}

unsigned int CCameraDriverDC1394::GetPan()
{
    return GeneralUintGet("Pan", &dc1394_get_pan);
}

void CCameraDriverDC1394::SetPan(unsigned int v)
{
    return GeneralUintSet("Pan", &dc1394_set_pan, v);
}

unsigned int CCameraDriverDC1394::GetTilt()
{
    return GeneralUintGet("Tilt", &dc1394_get_tilt);
}

void CCameraDriverDC1394::SetTilt(unsigned int v)
{
    return GeneralUintSet("Tilt", &dc1394_set_tilt, v);
}

unsigned int CCameraDriverDC1394::GetOpticalFilter()
{
    return GeneralUintGet("Optical Filter", &dc1394_get_optical_filter);
}

void CCameraDriverDC1394::SetOpticalFilter(unsigned int v)
{
    return GeneralUintSet("Optical Filter", &dc1394_set_optical_filter, v);
}

unsigned int CCameraDriverDC1394::GetCaptureSize()
{
    return GeneralUintGet("Capture Size", &dc1394_get_capture_size);
}

void CCameraDriverDC1394::SetCaptureSize(unsigned int v)
{
    return GeneralUintSet("Capture Size", &dc1394_set_capture_size, v);
}

unsigned int CCameraDriverDC1394::GetCaptureQuality()
{
    return GeneralUintGet("Capture Quality", &dc1394_get_capture_quality);
}

void CCameraDriverDC1394::SetCaptureQuality(unsigned int v)
{
    return GeneralUintSet("Capture Quality", &dc1394_set_capture_quality, v);
}

int CCameraDriverDC1394::str2dataspeed(const char* s)
{
    if (!strcmp("SPEED_100", s))
        return SPEED_100;
    if (!strcmp("SPEED_200", s))
        return SPEED_200;
    if (!strcmp("SPEED_400", s))
        return SPEED_400;
    if (!strcmp("SPEED_800", s))
        return SPEED_800;
    if (!strcmp("SPEED_1600", s))
        return SPEED_1600;
    if (!strcmp("SPEED_3200", s))
        return SPEED_3200;

    return -1;
}

int CCameraDriverDC1394::str2framerate(const char* s)
{
    if (!strcmp("FRAMERATE_1_875", s))
        return FRAMERATE_1_875;
    if (!strcmp("FRAMERATE_3_75", s))
        return FRAMERATE_3_75;
    if (!strcmp("FRAMERATE_7_5", s))
        return FRAMERATE_7_5;
    if (!strcmp("FRAMERATE_15", s))
        return FRAMERATE_15;
    if (!strcmp("FRAMERATE_30", s))
        return FRAMERATE_30;
    if (!strcmp("FRAMERATE_60", s))
        return FRAMERATE_60;
    if (!strcmp("FRAMERATE_120", s))
        return FRAMERATE_120;
    if (!strcmp("FRAMERATE_240", s))
        return FRAMERATE_240;
    
    return -1;
}

int CCameraDriverDC1394::str2mode(const char* s)
{
    //format 0
    if (!strcmp("MODE_160x120_YUV444", s))
        return MODE_160x120_YUV444;
    if (!strcmp("MODE_320x240_YUV422", s))
        return MODE_320x240_YUV422;
    if (!strcmp("MODE_640x480_YUV411", s))
        return MODE_640x480_YUV411;
    if (!strcmp("MODE_640x480_YUV422", s))
        return MODE_640x480_YUV422;
    if (!strcmp("MODE_640x480_RGB", s))
        return MODE_640x480_RGB;
    if (!strcmp("MODE_640x480_MONO", s))
        return MODE_640x480_MONO;
    if (!strcmp("MODE_640x480_MONO16", s))
        return MODE_640x480_MONO16;

    //format 1
    if (!strcmp("MODE_800x600_YUV422", s))
        return MODE_800x600_YUV422;
    if (!strcmp("MODE_800x600_RGB", s))
        return MODE_800x600_RGB;
    if (!strcmp("MODE_800x600_MONO", s))
        return MODE_800x600_MONO;
    if (!strcmp("MODE_1024x768_YUV422", s))
        return MODE_1024x768_YUV422;
    if (!strcmp("MODE_1024x768_RGB", s))
        return MODE_1024x768_RGB;
    if (!strcmp("MODE_1024x768_MONO", s))
        return MODE_1024x768_MONO;
    if (!strcmp("MODE_800x600_MONO16", s))
        return MODE_800x600_MONO16;
    if (!strcmp("MODE_1024x768_MONO16", s))
        return MODE_1024x768_MONO16;

    //format 2
    if (!strcmp("MODE_1280x960_YUV422", s))
        return MODE_1280x960_YUV422;
    if (!strcmp("MODE_1280x960_RGB", s))
        return MODE_1280x960_RGB;
    if (!strcmp("MODE_1280x960_MONO", s))
        return MODE_1280x960_MONO;
    if (!strcmp("MODE1600x1200_YUV422_", s))
        return MODE_1600x1200_YUV422;
    if (!strcmp("MODE_1600x1200_RGB", s))
        return MODE_1600x1200_RGB;
    if (!strcmp("MODE_1600x1200_MONO", s))
        return MODE_1600x1200_MONO;
    if (!strcmp("MODE_1280x960_MONO16", s))
        return MODE_1280x960_MONO16;
    if (!strcmp("MODE_1600x1200_MONO16", s))
        return MODE_EXIF;

    //format 6
    if (!strcmp("MODE_EXIF", s))
        return MODE_EXIF;

    //format 7
    if (!strcmp("MODE_FORMAT7_0", s))
        return MODE_FORMAT7_0;
    if (!strcmp("MODE_FORMAT7_1", s))
        return MODE_FORMAT7_1;
    if (!strcmp("MODE_FORMAT7_2", s))
        return MODE_FORMAT7_2;
    if (!strcmp("MODE_FORMAT7_3", s))
        return MODE_FORMAT7_3;
    if (!strcmp("MODE_FORMAT7_4", s))
        return MODE_FORMAT7_4;
    if (!strcmp("MODE_FORMAT7_5", s))
        return MODE_FORMAT7_5;
    if (!strcmp("MODE_FORMAT7_6", s))
        return MODE_FORMAT7_6;
    if (!strcmp("MODE_FORMAT7_7", s))
        return MODE_FORMAT7_7;

    return -1;
}

int CCameraDriverDC1394::str2triggermode(const char* s)
{
    if (!strcmp("TRIGGER_MODE_0", s))
        return TRIGGER_MODE_0;
    if (!strcmp("TRIGGER_MODE_1", s))
        return TRIGGER_MODE_1;
    if (!strcmp("TRIGGER_MODE_2", s))
        return TRIGGER_MODE_2;
    if (!strcmp("TRIGGER_MODE_3", s))
        return TRIGGER_MODE_3;

    return -1;
}

int CCameraDriverDC1394::str2format(const char* s)
{
    if (!strcmp("FORMAT_VGA_NONCOMPRESSED", s))
        return FORMAT_VGA_NONCOMPRESSED;
    if (!strcmp("FORMAT_SVGA_NONCOMPRESSED_1", s))
        return FORMAT_SVGA_NONCOMPRESSED_1;
    if (!strcmp("FORMAT_SVGA_NONCOMPRESSED_2", s))
        return FORMAT_SVGA_NONCOMPRESSED_2;
    if (!strcmp("FORMAT_STILL_IMAGE", s))
        return FORMAT_STILL_IMAGE;
    if (!strcmp("FORMAT_SCALABLE_IMAGE_SIZE", s))
        return FORMAT_SCALABLE_IMAGE_SIZE;

    return -1;
}

int CCameraDriverDC1394::str2encoding(const char* s)
{
    if (!strcmp("ENCODING_RGB", s))
        return ENCODING_RGB;
    if (!strcmp("ENCODING_BAYER", s))
        return ENCODING_BAYER;
    if (!strcmp("ENCODING_RGB48", s))
        return ENCODING_RGB48;
    if (!strcmp("ENCODING_UYVY", s))
        return ENCODING_UYVY;
    if (!strcmp("ENCODING_UYYVYY", s))
        return ENCODING_UYYVYY;
    if (!strcmp("ENCODING_Y", s))
        return ENCODING_Y;
    if (!strcmp("ENCODING_Y16", s))
        return ENCODING_Y16;

    return -1;
}


/*-----------------------------------------------------------------------
 *  query the camera to determine the Bayer pattern
 *-----------------------------------------------------------------------*/
int CCameraDriverDC1394::GetBayerPattern()
{
    
    quadlet_t qValue;
    
    GetCameraControlRegister( m_raw1394Handle, 
                              m_cameraNode,
                              0x1040,		/* Bayer Tile Mapping register */
                              &qValue );
    
    bayer_pattern_t pattern = BAYER_PATTERN_BGGR;
    switch( qValue )
    {
    case 0x42474752:	/* BGGR */
        pattern = BAYER_PATTERN_BGGR;
        break;
    case 0x47524247:	/* GRBG */
        pattern = BAYER_PATTERN_GRBG;
        break;
    case 0x52474742:  /* RGGB */
        pattern = BAYER_PATTERN_RGGB;
        break;
    case 0x47425247:	/* GBRG */
        pattern = BAYER_PATTERN_GBRG;
        break;
    case 0x59595959:	/* YYYY = BW */
        fprintf( stderr, "Camera is black and white\n" );
        Cleanup();
        return 1;
    default:
        fprintf( stderr, 
                 "Camera BAYER_TILE_MAPPING register has an unexpected value:\n"
                 "\t0x%x\n", qValue );
        //cleanup();
        return -1;
    }

    return pattern;

}

