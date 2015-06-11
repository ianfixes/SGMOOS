// DVLInstrument.cpp: implementation of the CDVLInstrument class.
//
//////////////////////////////////////////////////////////////////////
#include <MOOSLIB/MOOSLib.h>
#include <iostream>
#include <math.h>
#include <string.h>
using namespace std;

#include "DVLInstrument.h"


#ifndef PI
#define PI 3.141592653589793
#endif



bool DVLPortReadCallBack(char * pData, int nBufferLen,int nRead)
{
    return nRead==DVL_BINARY_ENSEMBLE_REPLY_LENGTH;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDVLInstrument::CDVLInstrument()
{
    //some sensible defaults (missionfile can overwrite this)
    SetAppFreq(10);
    SetCommsFreq(10);
    m_dfLastSummary = 0;
    m_dfAlignment = -45;
    m_dfHeadingCorrection = 0;
    m_bExposeVelocities = false;
    m_bOutputBinary         = false;
    m_bDebug = true;
    m_bClearToRx = true;
    
}

CDVLInstrument::~CDVLInstrument()
{
        
}


/////////////////////////////////////////////
///this is where it all happens..
bool CDVLInstrument::Iterate()
{
        
    if(GetData())
    {
        PublishData();
    }
        
    return true;
}


bool CDVLInstrument::OnStartUp()
{
    CMOOSInstrument::OnStartUp();
        
        
    //here we make the variables that we are managing
    double dfDVLPeriod = 0.2;
    //Earth Referenced Distance Traveled
    AddMOOSVariable("EARTH_EAST",       "SIM_X",  "DVL_X",          dfDVLPeriod);
    AddMOOSVariable("EARTH_NORTH",      "SIM_Y",  "DVL_Y",          dfDVLPeriod);
    AddMOOSVariable("EARTH_UP",         "SIM_Z",  "DVL_Z",          dfDVLPeriod);
    AddMOOSVariable("EARTH_FRESH",      "",       "DVL_FRESHNESS",  dfDVLPeriod);
        
    //Earth Referenced Velocity Data
    AddMOOSVariable("EARTH_VEL_EAST",   "",  "DVL_EARTH_VEL_X",      dfDVLPeriod);
    AddMOOSVariable("EARTH_VEL_NORTH",  "",  "DVL_EARTH_VEL_Y",      dfDVLPeriod);
    AddMOOSVariable("EARTH_VEL_UP",     "",  "DVL_EARTH_VEL_Z",      dfDVLPeriod);
    AddMOOSVariable("EARTH_VEL_STATUS", "",  "DVL_EARTH_VEL_STATUS", dfDVLPeriod);
    
    //AUV referenced Velocity Data
    AddMOOSVariable("AUV_VEL_EAST",     "",  "DVL_BODY_VEL_X",     dfDVLPeriod);   
    AddMOOSVariable("AUV_VEL_NORTH",    "",  "DVL_BODY_VEL_Y",     dfDVLPeriod);
    AddMOOSVariable("AUV_VEL_UP",       "",  "DVL_BODY_VEL_Z",     dfDVLPeriod);
    AddMOOSVariable("AUV_VEL_STATUS",   "",  "DVL_AUV_VEL_STATUS", dfDVLPeriod);
        
    //AUV referenced Attitude Data
    AddMOOSVariable("PITCH",    "SIM_PITCH",     "DVL_PITCH",    dfDVLPeriod);
    AddMOOSVariable("ROLL",     "SIM_ROLL",      "DVL_ROLL",     dfDVLPeriod);
    AddMOOSVariable("YAW",      "SIM_YAW",       "DVL_YAW",      dfDVLPeriod);
    AddMOOSVariable("HEADING",  "SIM_HEADING",   "DVL_HEADING",  dfDVLPeriod);
    AddMOOSVariable("SPEED",    "SIM_SPEED",     "DVL_SPEED",    dfDVLPeriod);        
    AddMOOSVariable("ALTITUDE", "SIM_ALTITUDE",  "DVL_ALTITUDE", dfDVLPeriod);        
        
    //Register for the variable that will provide for dynamic configuration
    //of the DVL
    AddMOOSVariable("UPDATE_CMD", "DVL_COMMAND", "DVL_UPDATE_REPLY",    dfDVLPeriod);        
        
    RegisterMOOSVariables();
        
    //get DVL alignment
    m_dfAlignment = -45;
    m_MissionReader.GetConfigurationParam("Alignment", m_dfAlignment);
    
    //get the Heading Correction
    m_dfHeadingCorrection = 0;
    m_MissionReader.GetConfigurationParam("HeadingCorrection", m_dfHeadingCorrection);
        
    //DVL can output in binary format, but we normally use ASCII
    //BINARY output is primarily for DVL characterization, as we keep
    //track of the individual beam ranges and beam statistic data
    m_MissionReader.GetConfigurationParam("BINARYOUTPUT", m_bOutputBinary);
        
    //add more variables if we are using the BINARY output
    if(m_bOutputBinary)
    {
        //beam ranges
        AddMOOSVariable("BM1_RANGE",  "",  "DVL_BM1_RANGE", dfDVLPeriod);
        AddMOOSVariable("BM2_RANGE",  "",  "DVL_BM2_RANGE", dfDVLPeriod);
        AddMOOSVariable("BM3_RANGE",  "",  "DVL_BM3_RANGE", dfDVLPeriod);
        AddMOOSVariable("BM4_RANGE",  "",  "DVL_BM4_RANGE", dfDVLPeriod);
        //beam velocities
        AddMOOSVariable("BM1_VEL",    "",  "DVL_BM1_VEL",   dfDVLPeriod);
        AddMOOSVariable("BM2_VEL",    "",  "DVL_BM2_VEL",   dfDVLPeriod);
        AddMOOSVariable("BM3_VEL",    "",  "DVL_BM3_VEL",   dfDVLPeriod);
        AddMOOSVariable("BM4_VEL",    "",  "DVL_BM4_VEL",   dfDVLPeriod);
        //beam correlation
        AddMOOSVariable("BM1_CORR",   "",  "DVL_BM1_CORR",  dfDVLPeriod);
        AddMOOSVariable("BM2_CORR",   "",  "DVL_BM2_CORR",  dfDVLPeriod);
        AddMOOSVariable("BM3_CORR",   "",  "DVL_BM3_CORR",  dfDVLPeriod);
        AddMOOSVariable("BM4_CORR",   "",  "DVL_BM4_CORR",  dfDVLPeriod);
        //beam amplitude
        AddMOOSVariable("BM1_AMP",    "",  "DVL_BM1_AMP",   dfDVLPeriod);
        AddMOOSVariable("BM2_AMP",    "",  "DVL_BM2_AMP",   dfDVLPeriod);
        AddMOOSVariable("BM3_AMP",    "",  "DVL_BM3_AMP",   dfDVLPeriod);
        AddMOOSVariable("BM4_AMP",    "",  "DVL_BM4_AMP",   dfDVLPeriod);
        //beam %good
        AddMOOSVariable("BM1_GOOD",   "",  "DVL_BM1_GOOD",  dfDVLPeriod);
        AddMOOSVariable("BM2_GOOD",   "",  "DVL_BM2_GOOD",  dfDVLPeriod);
        AddMOOSVariable("BM3_GOOD",   "",  "DVL_BM3_GOOD",  dfDVLPeriod);
        AddMOOSVariable("BM4_GOOD",   "",  "DVL_BM4_GOOD",  dfDVLPeriod);
        
        if(m_bDebug)
        {
            AddMOOSVariable("BM_RAW", "", "DVL_RAW", dfDVLPeriod);
        }
        //need to know when to stop reading how big the returned bundle is
        //m_Port.SetIsCompleteReplyCallBack(DVLPortReadCallBack);
    }
    
    
    
    //search the setup file for the Magnetic Offset
    GetMagneticOffset();
        
    //use the *pfn callback routine in the SerialPort to notify that we are interested
    //in using the prompt given back to us by the RDI
    //Error Message for Unrecognized Command: ERR 010:  UNRECOGNIZED COMMAND
    //Error Message for Out of bounds Command: ERR 045: PARAMETER OUT OF BOUNDS
    //Common to both error message strings is ERR
        
    SetPrompt(">");
    SetInstrumentErrorMessage("ERR");
    
    if(IsSimulateMode())
    {
        SetAppFreq(10);
        SetCommsFreq(8);
    }
    else
    {
        
        //try to open 
        if(!SetupPort())
        {
            return false;
        }
        
        //try 10 times to initialise sensor
        if(!InitialiseSensorN(10,"DVL"))
        {
            return false;
        }          
        
    }
    return true;
}



bool CDVLInstrument::OnNewMail(MOOSMSG_LIST &NewMail)
{
    CMOOSMsg Msg;
    
    
    if(m_Comms.PeekMail(NewMail,"DVL_SUMMARY_REQUIRED",Msg))
    {
        m_bExposeVelocities = MOOSStrCmp(Msg.m_sVal,"TRUE");
    }
    
    if(m_Comms.PeekMail(NewMail,"DVL_COMMAND",Msg))
    {
        MOOSTrace("..............new DVL_COMMAND: %s\n", Msg.m_sVal.c_str());
        //have to flag the Iterate() loop so no contention with serial port ensues
        //this will prevent GetData() from trying to interpret a response
        SetClearToRx( false );
        
        ExecuteDVLUpdateCommands(Msg);
        
        //it is now OK to GetData()
        SetClearToRx( true );
    }
    
    return UpdateMOOSVariables(NewMail);
}

bool CDVLInstrument::PublishData()
{
    if(m_bExposeVelocities && MOOSTime()-m_dfLastSummary>2.0)
    {
        m_dfLastSummary = MOOSTime();
        
        CMOOSVariable * pBVX = GetMOOSVar("AUV_VEL_EAST");
        CMOOSVariable * pBVY = GetMOOSVar("AUV_VEL_NORTH");
        
        string sText  = "NO DVL";
        
        if(pBVX && pBVY)
        {
            
            sText = MOOSFormat("DVL Body Vel Y = %7.2f X = %7.2f %s",
                               pBVY->GetDoubleVal(),
                               pBVX->GetDoubleVal(),
                               (pBVX->IsFresh() && pBVY->IsFresh()) ? "OK":"STALE");
            
        }
        
        MOOSDebugWrite(sText);
        
    }
    
    return PublishFreshMOOSVariables();
}



bool CDVLInstrument::OnConnectToServer()
{
    if(IsSimulateMode())
    {
        RegisterMOOSVariables();
    }
    
    m_Comms.Register("DVL_SUMMARY_REQUIRED",0.0);
    
    return true;
}

void CDVLInstrument::CreateInitialCommands(list<SendTermPair> &InitCmds)
{
    
    //serial port: 9600,8,N,1
    //InitCmds.push_back(SendTermPair("CB411\r", m_sPrompt.c_str()));
    InitCmds.push_back(SendTermPair("TC0000\r",m_sPrompt.c_str()));
    InitCmds.push_back(SendTermPair("TE00:00:00.00\r",m_sPrompt.c_str()));
    //time between pings is as fast as possible
    InitCmds.push_back(SendTermPair("TP00:00.00\r",m_sPrompt.c_str()));
    //use BM5 for shallow water, BM4 for deep water
    InitCmds.push_back(SendTermPair("BM5\r",m_sPrompt.c_str()));
    //tells DVL to guess depth - BF0 means DO NOT GUESS
    InitCmds.push_back(SendTermPair("BF0\r",m_sPrompt.c_str()));
    //max depth it should try to look for bottom in decimeters, e.g. 0400 = 400dm = 40m
    InitCmds.push_back(SendTermPair("BX2000\r",m_sPrompt.c_str()));
    //    InitCmds.push_back(SendTermPair("BI75\r",m_sPrompt.c_str()));
    //    InitCmds.push_back(SendTermPair("BF900\r",m_sPrompt.c_str()));
    InitCmds.push_back(SendTermPair("BP1\r",m_sPrompt.c_str()));
    InitCmds.push_back(SendTermPair("BK0\r",m_sPrompt.c_str()));
    InitCmds.push_back(SendTermPair("BS\r",m_sPrompt.c_str()));
    InitCmds.push_back(SendTermPair("EX00110\r",m_sPrompt.c_str()));
    InitCmds.push_back(SendTermPair("EZ1011101\r",m_sPrompt.c_str()));
    InitCmds.push_back(SendTermPair("ED0\r",m_sPrompt.c_str()));
        
    
    string sEA = MOOSFormat("EA%d\r",-(int)m_dfAlignment*100);
    string sEB = MOOSFormat("EB%d\r",(int)m_dfHeadingCorrection*100);
        
    InitCmds.push_back(SendTermPair(sEA,m_sPrompt.c_str()));
    InitCmds.push_back(SendTermPair(sEB,m_sPrompt.c_str()));
        
    if(m_bOutputBinary)
    {
        InitCmds.push_back(SendTermPair("PD5\r",m_sPrompt.c_str()));
        //we will be manually cycling the DVL
        //Binary data output has place3 set to 1
        InitCmds.push_back(SendTermPair("CF01010\r",m_sPrompt.c_str()));
    }
    else
    {
        InitCmds.push_back(SendTermPair("CF11010\r",m_sPrompt.c_str()));        
        InitCmds.push_back(SendTermPair("PD6\r",m_sPrompt.c_str()));
    }
    
    //for synchronization purposes
    //do not sleep between Pings
    InitCmds.push_back(SendTermPair("CL0\r",m_sPrompt.c_str()));
    //take dvl out of wait-on-sync mode
    InitCmds.push_back(SendTermPair("CX0\r",m_sPrompt.c_str()));
    
}


///////////////////////////////////////////////////////////////////////////
// here we initialise the sensor, giving it start up values
bool CDVLInstrument::InitialiseSensor()
{   
        //we shall need the diference between true north and magnetic north.
    GetMagneticOffset();
    
    //initialization commands will vary 
    TERMPAIRLIST InitCmds;
    CreateInitialCommands(InitCmds);
    
    //Wakeup the DVL
    GetDVLAttention();
    
    int nTries = 0;
    TERMPAIRLIST::iterator p;
    //send the initialization commands
    for(p = InitCmds.begin(); p != InitCmds.end();)
    {
        SendTermPair & rToDo = *p;
        
        //make it easy to read from the console
        //when debugging - this has nothing to do
        //with getting a good reply from the DVL
        MOOSPause(250);
        
        if(SendDVLCommandGoodReply(rToDo.sSend))
        {
            p++;
        }
        else
        {
            MOOSTrace("DVLInstrument::InitialiseSensor(): no good DVL reply\n");
            if(nTries++ > MAXTRIES)
                return false;
        }
    }
    
    //start up the pinging if we are in normal streaming mode
    if(!m_bOutputBinary)
    {   
        MOOSTrace("Begin Pinging\n");
        //reset the port to look for <CR>
        m_Port.SetTermCharacter('\r');
        //send the CS command
        StartPinging();
        //look for the reply
        string sReply;
        double dfWhen;
        ReadDVLReply(sReply, dfWhen);
        
        if( IsGoodReply(sReply, "CS") )
        {
            MOOSTrace("Update DVL: Pinging Restarted\n");
            return true;
        }
        else
        {
            MOOSTrace("Update DVL: Pinging Failed - CS missing\n");
            return false;
        }
        
    }
    
    return true;
}


/**
 *The DVL returns data about the Attitude of the vehicle: Pitch, Roll, Heading.
 *Currently using it in PD6 mode, with Bottom Track enabled, thus we will receive
 *information about the velocity in XYZ w.r.t. bottom.  The returned strings are 
 *recognized in the ParseRDIReply(string &sWhat) method, thus eliminating the need
 *of a 'for()' loop to grab all data in this method. 
 */

bool CDVLInstrument::GetData()
{
    if(!IsSimulateMode())
    {
        //only get data when we are not ExecuteDVLUpdateCommands
        if( !m_bClearToRx )
            return true;
        
        if(m_bOutputBinary)
        {
            return ReadHexDVLOutput();      
        }
        else
        {
            //get the ASCII data and parse
            string sWhatReply;
            double dfWhen;
            
            if(m_Port.IsStreaming())
            {
                double dfNow = MOOSTime();
                double dfSkew = 0.5;
                while( ReadDVLReply(sWhatReply, dfWhen) )
                {
                    //check that we are not publishing old info
                    if( fabs(dfNow - dfWhen) < dfSkew )
                        ParseRDIReply(sWhatReply);
                    else
                        MOOSTrace("GetData(): old {%3.3f}=={%s}", fabs(dfNow - dfWhen), sWhatReply.c_str());
                }
            }
            else
            {
                MOOSTrace("DVL Must be streaming if not Binary...\n");
                return false;   
            }

            /*
            if(m_Port.IsStreaming())
            {
                if ( !ReadDVLReply(sWhatReply, dfWhen) )
                    return false;
            }
            else
            {
                MOOSTrace("DVL Must be streaming if not Binary...\n");
                
                return false;
                
            }
            //see what has come back
            ParseRDIReply(sWhatReply);
            */
        }  
                
    }
    else
    {
        //in simulated mode there is nothing to do..all data
        //arrives via comms.
    }
    
    return true;
    
}

double CDVLInstrument::Magnetic2True(double dfMagnetic)
{
    return dfMagnetic+m_dfMagneticOffset;
}

double CDVLInstrument::True2Yaw(double dfTrueHeading)
{
    return -dfTrueHeading;
}

/**
 * This method interprets the RDI strings taken from the serial port.  Be aware
 * that changing the settings of the sensor will change what strings it returns to us!
 * This method only returns attitude data at the moment.  The DVLInstrument will have to 
 * Publish new variables in order to accomodate the bottom track related information.
 * RDI Setup: 
 * +Output Data format PD6
 * +No Water Reference Data
 * +Bottom Track enabled 
 * +Velocities are in mm/s, need to convert this to m/s for our world
 */
bool CDVLInstrument::ParseRDIReply(string &sReply)
{
    string sCopy = sReply;
    string sWhat = MOOSChomp(sReply,",");
        
    double dfTimeNow = MOOSTime();
        
    if(sWhat==":SA")
    {
        //Vehicle Attitude Information
        double dfPitch   = atof(MOOSChomp(sReply,",").c_str());
        double dfRoll    = atof(MOOSChomp(sReply,",").c_str());
        double dfHeading = atof(MOOSChomp(sReply,",").c_str());
        
        SetMOOSVar("PITCH",PI/180.0*dfPitch,dfTimeNow);
        SetMOOSVar("ROLL", PI/180.0*dfRoll,  dfTimeNow);
        
        //look after the magnetic offset - gives us true north
        double dfHeadingAngle = Magnetic2True(dfHeading);
        SetMOOSVar("HEADING", dfHeadingAngle,  dfTimeNow);
        
        //put heading into the yaw domain
        dfHeadingAngle = True2Yaw(dfHeadingAngle);
        //look after the yaw in rad
        dfHeadingAngle *= PI/180.0;
        //and take care of wrapping
        dfHeadingAngle = MOOS_ANGLE_WRAP(dfHeadingAngle);
        
        SetMOOSVar("YAW", dfHeadingAngle, dfTimeNow);
        
    }
    else if (sWhat == ":BI")
    {
        //Bottom Track, Instrument Referenced Velocity Data
        //
        //X axis: Beam1 Beam2 xdcr movement relative to bottom
        //Y axis: Beam4 Beam3 xdcr movement relative to bottom
        //Z axis: Transducre movement away from bottom
        //Error is the error velocity
        //The status is either 'A' = Good or 'V' = Bad

        int nXaxisVel    = (int)atof(MOOSChomp(sReply,",").c_str());
        int nYaxisVel    = (int)atof(MOOSChomp(sReply,",").c_str());
        int nZaxisVel    = (int)atof(MOOSChomp(sReply,",").c_str());
        int nErrorVel    = (int)atof(MOOSChomp(sReply,",").c_str());

        //avoid compiler warnings for unused vars
        (void) nXaxisVel;
        (void) nYaxisVel;
        (void) nZaxisVel;
        (void) nErrorVel;

        string sStatus   = MOOSChomp(sReply,",");
        
        if (sStatus == "A")
        {
            //SetMOOSVar here
            //Convert to m/s
        }
        else if (sStatus == "V")
        {
            //alert to failure
        }
        
    }
    else if (sWhat == ":BS")
    {
        //Bottom Track, Ship Referenced Velocity Data
        //
        //RDI defines positive as Right Handed with Y extending through the Bow
        //and X extending out through Starboard.  This makes Z positive upwards,
        //extending away from the bottom.
        //
        //Transverse is the Port-Stbd ship movement
        //Longitudinal is the Aft-Fwd ship movement
        //Normal is the ship movement away from bottom
        //The status is either 'A' = Good or 'V' = Bad
        int nTransVel    = (int)atof(MOOSChomp(sReply,",").c_str());
        int nLongVel     = (int)atof(MOOSChomp(sReply,",").c_str());
        int nNormalVel   = (int)atof(MOOSChomp(sReply,",").c_str());
        string sStatus   = MOOSChomp(sReply,",");
        
        if (sStatus == "A")
        {
            //SetMOOSVar here
            //Convert to m/s
            SetMOOSVar("AUV_VEL_EAST",nTransVel*.001,dfTimeNow);
            SetMOOSVar("AUV_VEL_NORTH",nLongVel*.001,dfTimeNow);
            SetMOOSVar("AUV_VEL_UP",nNormalVel*.001,dfTimeNow);
            SetMOOSVar("AUV_VEL_STATUS",sStatus,dfTimeNow);
        }
        else if (sStatus == "V")
        {
            //alert to failure
            SetMOOSVar("AUV_VEL_STATUS",sStatus,dfTimeNow);
        }
        
        
    }
    else if (sWhat == ":BE")
    {
        //Bottom Track, Earth Referenced Velocity Data
        //
        //East is the ADCP movement to East
        //North is the ADCP movement to North
        //Upward is the ADCP movement to Surface
        //The status is either 'A' = Good or 'V' = Bad
        int nEastVel     = (int)atof(MOOSChomp(sReply,",").c_str());
        int nNorthVel    = (int)atof(MOOSChomp(sReply,",").c_str());
        int nUpwardVel   = (int)atof(MOOSChomp(sReply,",").c_str());
        string sStatus   = MOOSChomp(sReply,",");
        
        if (sStatus == "A")
        {
            //SetMOOSVar here
            //Convert to m/s
            SetMOOSVar("EARTH_VEL_NORTH",nNorthVel*.001,dfTimeNow);
            SetMOOSVar("EARTH_VEL_EAST",nEastVel*.001,dfTimeNow);
            SetMOOSVar("EARTH_VEL_UP",nUpwardVel*.001,dfTimeNow);
            SetMOOSVar("EARTH_VEL_STATUS",sStatus,dfTimeNow);
        }
        else if (sStatus == "V")
        {
            //alert to failure
            SetMOOSVar("EARTH_VEL_STATUS",sStatus,dfTimeNow);
        }
        
    }
    else if (sWhat == ":BD")
    {
        //Bottom Track, Earth Referenced Distance Data
        //
        //East is the distance East in meters
        //North is the distance North in meters
        //Upward is the distance Upward in meters
        //Bottom is the range to bottom in meters
        //Time is the Time since last good velocity estimate in seconds
        double dfEastDist      = atof(MOOSChomp(sReply,",").c_str());
        double dfNorthDist     = atof(MOOSChomp(sReply,",").c_str());
        double dfUpDist        = atof(MOOSChomp(sReply,",").c_str());
        double dfBottomDist    = atof(MOOSChomp(sReply,",").c_str());
        double dfTimeSinceGood = atof(MOOSChomp(sReply,",").c_str());
        
        if(dfBottomDist!=0 && dfTimeSinceGood<3.0)
        {
            //no status here so use time and altitude as measure of success
            SetMOOSVar("ALTITUDE",dfBottomDist,dfTimeNow);
            SetMOOSVar("EARTH_NORTH",dfNorthDist,dfTimeNow);
            SetMOOSVar("EARTH_EAST",dfEastDist,dfTimeNow);
            SetMOOSVar("EARTH_UP",dfUpDist,dfTimeNow);
            SetMOOSVar("EARTH_FRESH",dfTimeSinceGood,dfTimeNow);
        }                   
    }
    else if(sWhat == ":TS")
    {
        MOOSChomp(sReply,",");
        
        double dfSalinity = atof(MOOSChomp(sReply,",").c_str());
        double dfTemp = atof(MOOSChomp(sReply,",").c_str());
        double dfDepth = atof(MOOSChomp(sReply,",").c_str());
        double dfSoundSpeed = atof(MOOSChomp(sReply,",").c_str());
        double dfBIT = atof(MOOSChomp(sReply,",").c_str());

        //avoid compiler warnings for unused vars
        (void) dfSalinity;
        (void) dfTemp;
        (void) dfDepth;
        (void) dfSoundSpeed;
        (void) dfBIT;
    }
    else
    {
        //This was a response we are not expecting
        MOOSTrace("Unknown RDI reply %s\n",sWhat.c_str());
        return false;
    }
    
    //we are here so we processed the string
    if(PublishRaw())
    {
        //yep user want to see raw data...
        m_Comms.Notify("DVL_RAW_ASCII",sCopy);
    }
    
    return true;
}


bool CDVLInstrument::IsGoodReply(string sReply, string sTok)
{
    char * pPrompt = NULL;
    char * pError = NULL;
    bool bFoundToken = true;
    
        
    if(sTok.size() > 0)
    {
        //trim the trailing <CR> from commands
        MOOSRemoveChars(sTok,"\r\n");
        
        bFoundToken = (sReply.find(sTok) == string::npos) ? false : true;       
        if( m_Port.IsVerbose() )
        {
            MOOSTrace("DVLInstrument::IsGoodReply?: {%s==%s} FoundToken: %d\n", 
                      sReply.c_str(), sTok.c_str(), bFoundToken);
        }
        
        //now check for the "CS" - only command to not have '>' after it
        //thus, making the logic that checks for prompts and errors not applicable
        if( sTok == "CS" )
            return bFoundToken;
        
    }
    
    //look for the prompt and an error
    //keep track of where each resides
    pPrompt = strstr((char*)sReply.c_str(), m_sPrompt.c_str());
    pError = strstr((char*)sReply.c_str(), m_sInstrumentErrorMessage.c_str());
    
    if( m_Port.IsVerbose() )
    {
        MOOSTrace("DVLInstrument::IsGoodReply?: {Reply:%s}\n {prompt:%d}\n {error:%d}\n", 
                  sReply.c_str(), pPrompt, pError);
    }
    if((pError != NULL) && (pPrompt != NULL))//prompt, error
        return false;
    else if((pError != NULL) && (pPrompt == NULL))//no prompt, error
        return false;
    else if((pError == NULL) && (pPrompt == NULL)) //no prompt, no error
        return false;
    else//prompt, no error
    {
        //we have seen the prompt, but no Error string
        //so also make sure we have seen the Token of interest
        if(sTok.size() > 0)
            return bFoundToken;
    }
    
    return true;
}


bool CDVLInstrument::ExecuteDVLUpdateCommands(CMOOSMsg &Msg)
{
#define FRESH_UPDATE 30.0
    string sCmdUpdateStatus;
    double dfTimeNow = MOOSTime();
    
    //only interested in the string message of comma separated
    //values for the DVL.
    string sNewCmdString = Msg.m_sVal;
    if( m_Port.IsVerbose() )
        MOOSTrace("Update DVL: Cmd - [%s] - Size - %d  \n", sNewCmdString.c_str(), sNewCmdString.size());
    
    //check for skewedness
    if(NewCommandsAreSkewed(Msg, dfTimeNow, sNewCmdString))
        return false;
    
    //Stop the DVL from streaming and get it's attention
    //note SoftwareBreak mode reduces time to 1.75sec
    //XXX: ugly hack - have to scan for the CX0 command to 
    //XXX: use the hw BREAK to re-wake the DVL - instead of sw BREAK
    if( sNewCmdString.find("CX0") != string::npos )
    {
        //hardware BREAK
        if(!GetDVLAttention())
            return false;
    }
    else
    {
        //software BREAK
        if(!GetDVLAttention(false))
            return false;
    }
    
    
    if( m_Port.IsVerbose() )
        MOOSTrace("ExecuteDVLUpdateCommands(): Have DVL Attention!\n");
    
    //send the commands
    if(SendDVLUpdatedCommands(sCmdUpdateStatus, sNewCmdString))
    {
        //reset the Termination character
        m_Port.SetTermCharacter('\r');
        
        //start the sensor pinging again
        if(!m_bOutputBinary)
        {
            StartPinging();
            //look for the reply
            string sReply;
            double dfWhen;
            ReadDVLReply(sReply, dfWhen);
            
            if( IsGoodReply(sReply, "CS") )
                MOOSTrace("Update DVL: Pinging Restarted\n");
            else
                MOOSTrace("Update DVL: Pinging Failed - CS missing\n");
            
            
            SetMOOSVar("UPDATE_CMD", sCmdUpdateStatus, dfTimeNow);
        }
    }
    else
    {
        MOOSTrace("Update DVL: Failure - Pinging Not Restarted\n");
        SetMOOSVar("UPDATE_CMD", sCmdUpdateStatus, dfTimeNow);
    }
    
    return PublishFreshMOOSVariables();
}

/**
 *Starts the DVL pinging by sending "CS" command followed by <CR>
 */
bool CDVLInstrument::StartPinging()
{
    
    SendTermPair keepPinging("CS\r", m_sPrompt);
    
    return SendDVLCommand(keepPinging.sSend);
}

/**
 *Cycles through the string of new commands and checks for a successful update
 *
 */
bool CDVLInstrument::SendDVLUpdatedCommands(string &sCmdUpdateStatus, string sNewCmdString)
{
    string sMOOSChompResult = MOOSChomp(sNewCmdString, ",");
    string sWhatReply;
    bool bSuccess = true;
    int nTries = 0;
    
    while(!sMOOSChompResult.empty())
    {
        //take action here and talk to the DVL
        //have to add a <CR> to the command
        string sDVLUpdateFormattedCmd = sMOOSChompResult + "\r";
        
        //try at least 10 times
        //because if we are here, then we at least have DVL's attention
        while( !SendDVLCommandGoodReply(sDVLUpdateFormattedCmd))
        {
            //chill - if we are talking to it while it is resetting
            //after a BREAK, this might take up to 700ms to come back
            MOOSPause(100);
            
            if( nTries++ > MAXTRIES )
            {
                MOOSTrace("Update DVL: Failure - %s\n", sDVLUpdateFormattedCmd.c_str());
                
                //keep track of the failure
                sCmdUpdateStatus += sMOOSChompResult + ": FAIL; ";
                if(bSuccess)
                    bSuccess = !bSuccess;
                break;
            }
        }
        
        MOOSTrace("Update DVL: Succesful Update - %s\n", sDVLUpdateFormattedCmd.c_str());
        
        //keep track of the success
        sCmdUpdateStatus += sMOOSChompResult + ": OK; ";            
        
        sMOOSChompResult = MOOSChomp(sNewCmdString, ",");
        
    }
    
    return bSuccess;
}

/**
*Make sure that the commands being sent are not skewed commands.
*that is left over from a database write from a while ago
*and that the user has actually entered in a new command.
*Should also eventually check this against allowable commands to be
*sent to the device.
*/
bool CDVLInstrument::NewCommandsAreSkewed(CMOOSMsg & Msg, double dfTimeNow, string sNewCmdString)
{
    double dfSkewTime;
    string sCmdUpdateStatus;
        
    //sanity checking
    if( (sNewCmdString.size() == 0) )
    {
        sCmdUpdateStatus = "Update DVL: Cmd SIZE = 0 - Aborted Update\n";
        MOOSTrace("%s", sCmdUpdateStatus.c_str());
        SetMOOSVar("UPDATE_CMD",sCmdUpdateStatus,MOOSTime());
        return true;
    }
    
    //do a skew check:
    
    if( !Msg.IsSkewed(dfTimeNow, &dfSkewTime) )
    {
        //we are OK anyways, so go ahead
        MOOSTrace("Update DVL: Time Stamp - %f\n", Msg.m_dfTime);
        MOOSTrace("Update DVL: MOOS Time Stamp - %f\n", dfTimeNow);
        
        return false;
    }
    else
    {
        //because of the amount of time a Sonar command might take,
        //we have to be very forgiving here about our skew tolerance
        if( dfSkewTime < FRESH_UPDATE )
        {
            //we are OK anyways, so go ahead
            MOOSTrace("Update DVL: Time Stamp skewed, but OK - %f\n", Msg.m_dfTime);
            MOOSTrace("Update DVL: MOOS Time Stamp - %f\n", dfTimeNow);
            return false;
        }
        else
        {
            sCmdUpdateStatus = "Update DVL: Cmd [" + sNewCmdString + "] SKEWED - Aborted Update\n";
            MOOSTrace("%s\n SkewTime: %f\n", sCmdUpdateStatus.c_str(), dfSkewTime); 
            SetMOOSVar("UPDATE_CMD",sCmdUpdateStatus,MOOSTime());
            return true;
        }
    }
    
}

/**
 *Either gets the prompt from the DVL in "Software Break" mode, by sending '+++'
 *or sends a Break (TX low for 300ms) over the serial line and check to see 
 *if the DVL replies with its prompt.
 */
bool CDVLInstrument::GetDVLAttention(bool bHWBreak)
{
    string sWhatReply;
    double dfWhen;
    int nTries = 0;    
    
    //change the Termination Character we are looking for
    //and stop the DVL from talking
    m_Port.SetTermCharacter(m_sPrompt.c_str()[0]);
    //check reply...
    while(nTries++ < MAXTRIES)
    {
        
        if(bHWBreak)
        {
            //When streaming data back, a single Break() will also capture
            //gobs of :BS data, which might erroneously be parsed
            //so to avoid this, we Break() twice to ensure the last set of
            //characters into a GetLatest() call via ReadDVLReply will relate
            //to the Break() call
            m_Port.Break();
            //wait 1.25sec to let it present us with the prompt
            //this is slight overkill, as the highest lag i have seen is 1.1sec after
            //a BREAK is sent
            MOOSPause(1250);
            
        }
        else
        {
            //note, no <CR>
            string sSWBreak = "+++";
            SendDVLCommand(sSWBreak);
            //DVL takes approx 40ms to reply with [RESTART]
            //but around 950-1000ms to show the prompt, so being safe
            MOOSPause(1250);
        }
        
        //suck up the data that might have been sent after DVL was "BREAK"d

        ReadDVLReply(sWhatReply, dfWhen);
        //bizarre, but we can get data out of sequence from the
        //DVL - i.e. an :SA will show up after the BREAK appears
        //so, the loop is here to purge our local (i.e. vehicle side) serialport
        //buffers, and we keep track of whether we saw the BREAK sentence
        //which indicates, in the end, that the DVL is ready to
        //hear from us
        if(IsGoodReply(sWhatReply))
            return true;
        else
            MOOSTrace("Update DVL: No Prompt Found - Retrying Break\n");
        
    }
    /*
    //suck up the data
    while( ReadDVLReply(sWhatReply, dfWhen) );
    //final break
    m_Port.Break();
        
    //check reply...
    while(nTries++ < MAXTRIES)
    {  
        //wait 1.25sec to let it present us with '>'
        MOOSPause(1250);
                
                ReadDVLReply(sWhatReply, dfWhen);
                
        if(IsGoodReply(sWhatReply))
        {
                        return true;
        }
        else
        {
            MOOSTrace("Update DVL: No Prompt Found - Retrying Break\n");
            m_Port.Break();
        }
    }*/
        
    MOOSTrace("CDVLInstrument::GetDVLAttention(): Check Cable Connection\n");
    
    return false;
    
}

bool CDVLInstrument::ReadHexDVLOutput()
{
    
    string sWhatReply;
    double dfWhen;
    
    //make sure we are looking for the '>'
    m_Port.SetTermCharacter(m_sPrompt.c_str()[0]);
    
    //ping the DVL
    StartPinging();
    //read the Hex output
    if( !ReadDVLReply(sWhatReply, dfWhen, 100) )
        return false;
    //we are always returned the CS in the beginning of the output
    while(sWhatReply.find("CS") != string::npos)
        MOOSChomp(sWhatReply,"CS");
    
    //and the prompt is appended at the end of the ASCII stream
    MOOSRemoveChars(sWhatReply, m_sPrompt.c_str());
    
    //only parse if the string contains information
    if ( sWhatReply.size() > 0 )
    {
        ParseHexRDIReply(sWhatReply);
        return true;
    }
    else
    {
        MOOSTrace("CDVLInstrument::ReadHexDVLOutput(): reply is zero size\n");
        return false;
    }
    
}

void CDVLInstrument::ParseHexRDIReply(string &sReply)
{
    //have this object parse the hex reply
    CHexPD5Reader HexReader(sReply);
    signed char * HexBuffer = HexReader.GetHexBuffer();
    
    double dfTimeNow = MOOSTime();
    if(m_bDebug)
    {
        SetMOOSVar("BM_RAW",sReply,dfTimeNow);
    }
    //for PD5 processing
    //bottom vel
    double dfByteTotal = ((HexBuffer[3]) << 8) + (HexBuffer[2]);
    
    if( m_Port.IsVerbose() )
        MOOSTrace("ByteTotal= %2.2f\n", dfByteTotal);
    
    double dfXVel = (HexBuffer[6] << 8) + HexBuffer[5];
    double dfYVel = (HexBuffer[8] << 8) + HexBuffer[7];
    double dfZVel = (HexBuffer[10] << 8) + HexBuffer[9];
    double dfEVel = (HexBuffer[12] << 8) + HexBuffer[11];
    
    SetMOOSVar("BM1_VEL",dfXVel,dfTimeNow);
    SetMOOSVar("BM2_VEL",dfYVel,dfTimeNow);
    SetMOOSVar("BM3_VEL",dfZVel,dfTimeNow);
    SetMOOSVar("BM4_VEL",dfEVel,dfTimeNow);
    
    //bottom range
    double dfBM1Range = (HexBuffer[14] << 8) + HexBuffer[13];
    double dfBM2Range = (HexBuffer[16] << 8) + HexBuffer[15];
    double dfBM3Range = (HexBuffer[18] << 8) + HexBuffer[17];
    double dfBM4Range = (HexBuffer[20] << 8) + HexBuffer[19];
    
    SetMOOSVar("BM1_RANGE",dfBM1Range,dfTimeNow);
    SetMOOSVar("BM2_RANGE",dfBM2Range,dfTimeNow);
    SetMOOSVar("BM3_RANGE",dfBM3Range,dfTimeNow);
    SetMOOSVar("BM4_RANGE",dfBM4Range,dfTimeNow);
    
    //roll pitch yaw
    double dfPitch=((HexBuffer[49]) << 8) + (HexBuffer[48]);
    double dfRoll=((HexBuffer[51]) << 8) + (HexBuffer[50]);
    double dfHeading=((HexBuffer[53]) << 8) + (HexBuffer[52]);
    
    SetMOOSVar("PITCH",dfPitch/100.0,dfTimeNow);
    SetMOOSVar("ROLL",dfRoll/100.0,  dfTimeNow);
    SetMOOSVar("HEADING",fabs(dfHeading/100.0),  dfTimeNow);
    SetMOOSVar("YAW",fabs(dfHeading/100.0)*(PI/180.0),dfTimeNow);
    
    return;
    
}

bool CDVLInstrument::SendDVLCommand(string &sCommand)
{
    m_Port.Write((char *)sCommand.c_str(), sCommand.size());
    //DVL echoes back all characters sent to it
    //so we need to wait after talking so that a reply
    //ends up in the SerialPort when we go to call m_Port.GetLatest
    //otherwise we will pop() off old replies and get confused during interpretation
    MOOSPause(100);
    
    return true;
}

bool CDVLInstrument::ReadDVLReply(string &sReply, double &dfTime, int nWait)
{
    return m_Port.GetLatest(sReply, dfTime);
}

bool CDVLInstrument::SendDVLCommandGoodReply(string &sCommand)
{
    
    string sWhatReply;
    double dfWhen;
    
    if( m_Port.IsVerbose() )
        MOOSTrace("SendDVLCommandGoodReply() Sending: [%s]\n", sCommand.c_str());
    
    SendDVLCommand(sCommand);
    
    if( !ReadDVLReply(sWhatReply, dfWhen) )
        return false;
    else
        return IsGoodReply(sWhatReply, sCommand);
}


void CDVLInstrument::SetClearToRx(bool bRxOk)
{
    m_bClearToRx = bRxOk;
}

/////////////////////////INNER CLASS////////////////////////////

CDVLInstrument::CHexPD5Reader::CHexPD5Reader(string sHexString)
{
    m_sHexString = sHexString;
    ParseHexString();
}

CDVLInstrument::CHexPD5Reader::~CHexPD5Reader()
{
    
}

void CDVLInstrument::CHexPD5Reader::SetHexString(string sHexString)
{
    m_sHexString = sHexString;
}

void CDVLInstrument::CHexPD5Reader::ParseHexString()
{
    int j = 0;
    for (unsigned int i = 0; i < m_sHexString.size(); i+=2)
    {
        unsigned char cA = m_sHexString.c_str()[i];
        unsigned char cB = m_sHexString.c_str()[i+1];
        
        switch(cA){
        case 'A':
            cA=10*16;
            break;
        case 'B':
            cA=11*16;
            break;
        case 'C':
            cA=12*16;
            break;
        case 'D':
            cA=13*16;
            break;
        case 'E':
            cA=14*16;
            break;
        case 'F':
            cA=15*16;
            break;
        default:
            if (cA < 58 && cA >= 48)
            {
                cA-=48;
            }
            cA=cA*16;
            break;
        }

        switch(cB){
        case 'A':
            cB=10;
            break;
        case 'B':
            cB=11;
            break;
        case 'C':
            cB=12;
            break;
        case 'D':
            cB=13;
            break;
        case 'E':
            cB=14;
            break;
        case 'F':
            cB=15;
            break;
        default:
            if (cB < 58 && cB >= 48)
            {
                cB-=48;
            }
            
            break;
            
        }
        
        m_pHexBuffer[j++] = cA + cB;
    }       
}

signed char* CDVLInstrument::CHexPD5Reader::GetHexBuffer()
{
        return m_pHexBuffer;
}

