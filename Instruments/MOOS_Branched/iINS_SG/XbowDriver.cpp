// XbowDriver.cpp: implementation of the CXbowDriver class.
//
//////////////////////////////////////////////////////////////////////

#include "XbowDriver.h"
#include <iostream>
#include <math.h>

#define CROSSBOW_POLLED_ANGLE_MODE_REPLY_LENGTH 30

bool XbowPortReadCallBack(char * pData, int nBufferLen,int nRead)
{
    return nRead==CROSSBOW_POLLED_ANGLE_MODE_REPLY_LENGTH;
}

CXbowDriver::CXbowDriver()
{
    m_nTempCnt = 0;
    m_dfTemp = -1;
}

CXbowDriver::~CXbowDriver()
{

}


bool CXbowDriver::GetData()
{

    //here we actually access serial ports etc
    if(m_pPort->IsStreaming())
    {
        MOOSTrace("Crossbow must not be streaming\n");
        return false;
    }
    
    //ask the crossbow for a reading
    m_pPort->Write("G",1);
    
    string sWhat;
    unsigned char Reply[CROSSBOW_POLLED_ANGLE_MODE_REPLY_LENGTH];
    
    //note local call back invoked here to specify termination
    int nRead = m_pPort->ReadNWithTimeOut((char*)Reply,sizeof(Reply));

    
    if(nRead == CROSSBOW_POLLED_ANGLE_MODE_REPLY_LENGTH)
    {
        if(Reply[0]!=255)
        {
            MOOSTrace("Unexpected Header in CrossBow reply\n");
        }
		
	//angles
        short nRoll  = (Reply[1]<<8) + Reply[2];
        short nPitch = (Reply[3]<<8) + Reply[4];
        short nYaw   = (Reply[5]<<8) + Reply[6];

	//This is roll, pitch and yaw for the DMU-AHRS, which is rotated
	//relative to the vehicle body. It has Z down and X out the tail.
        double df_deg_CBRoll   = nRoll*180.0/pow(2.0,15.0);
        double df_deg_CBPitch  = nPitch*180.0/pow(2.0,15.0);
        double df_deg_CBYaw    = nYaw*180.0/pow(2.0,15.0);
		
        //account for alignment of crossbow in vehicle frame
        //this is the angle measured from the vehicle center line(y)
        double dfHeading = df_deg_CBYaw+m_dfVehicleYToINSX;
        
        //now correct for magnetic offset
        dfHeading+=m_dfMagneticOffset;
        
        //convert to Yaw..
	//Notice that Heading is in DEGREES, positive CLOCKWISE
	//while Yaw is in RADIANS, positive COUNTERCLOCKWISE
        double dfYaw = -dfHeading*PI/180.0;
        dfYaw = MOOS_ANGLE_WRAP(dfYaw);
        
		
	//initialise transformed angles (in degrees)
	double df_deg_TRoll  = df_deg_CBRoll;
	double df_deg_TPitch = df_deg_CBPitch;
		
	if( m_dfVehicleYToINSX == 0 )
	{
	    //Crossbow roll and vehicle body roll match.
	    df_deg_TRoll = df_deg_TRoll;
	    //same for Pitch.
	    df_deg_TPitch = df_deg_TPitch;
	}
	else if (m_dfVehicleYToINSX == 180)
	{
	    df_deg_TRoll = -df_deg_TRoll;
	    df_deg_TPitch = -df_deg_TPitch;
	}
	else
	{
	    MOOSTrace("Bad 'TWIST' value (use 0 or 180)!\r\n");
	    return false;
	}
		
        //look after pitch
        double dfPitch = MOOSDeg2Rad(df_deg_TPitch);
        
        //look after roll
        double dfRoll = MOOSDeg2Rad(df_deg_TRoll);
        
        
        //find the temperature every so often
        m_nTempCnt++;

        if((m_nTempCnt % 100) == 0)
        {
            short nTemp   = ( Reply[25] << 8 ) + Reply[26];
            m_dfTemp = 44.4 * ( ((double)nTemp * 5.0/4096.0) - 1.375); 
            CMOOSMsg Temperature(MOOS_NOTIFY, "INS_TEMPERATURE", m_dfTemp);
	    m_Notifications.push_back(Temperature);
        }
        
        //notify the MOOS
        CMOOSMsg Heading(MOOS_NOTIFY, "INS_HEADING", dfHeading);
	m_Notifications.push_back(Heading);
	CMOOSMsg Yaw(MOOS_NOTIFY, "INS_YAW", dfYaw);
	m_Notifications.push_back(Yaw);
	CMOOSMsg Pitch(MOOS_NOTIFY, "INS_PITCH", dfPitch);
	m_Notifications.push_back(Pitch);
	CMOOSMsg Roll(MOOS_NOTIFY, "INS_ROLL", dfRoll);
	m_Notifications.push_back(Roll);
		
	//rates
	short nRollRate  = (Reply[7]<<8) + Reply[8];//p-dot
        short nPitchRate = (Reply[9]<<8) + Reply[10];//q-dot
        short nYawRate   = (Reply[11]<<8) + Reply[12];//r-dot

	//xbow calib constant
	double RR = 100.0;//deg/sec

	double dfRollRate  = nRollRate * RR * 1.5/pow(2.0,15);
	double dfPitchRate = nPitchRate * RR * 1.5/pow(2.0,15);
	double dfYawRate   = nYawRate * RR * 1.5/pow(2.0,15);

	//note X and Y axes are switched
	CMOOSMsg RollRate(MOOS_NOTIFY, "INS_ROLLRATE_Y", dfRollRate);
	m_Notifications.push_back(RollRate);
	CMOOSMsg PitchRate(MOOS_NOTIFY, "INS_ROLLRATE_X", dfPitchRate);
	m_Notifications.push_back(PitchRate);
	CMOOSMsg YawRate(MOOS_NOTIFY, "INS_ROLLRATE_Z", dfYawRate);
	m_Notifications.push_back(YawRate);

	//accelerations
        short nXAccel = (Reply[13]<<8) + Reply[14];//v-dot
        short nYAccel = (Reply[15]<<8) + Reply[16];//u-dot
        short nZAccel = (Reply[17]<<8) + Reply[18];//w-dot
		
	//xbow calib constant
	double GR = 2.0;//G's
	double dfXAccel = nXAccel * GR * 1.5/pow(2.0,15);
	double dfYAccel = nYAccel * GR * 1.5/pow(2.0,15);
	double dfZAccel = nZAccel * GR * 1.5/pow(2.0,15);

	//note X and Y axes are switched
	CMOOSMsg XAccel(MOOS_NOTIFY, "INS_ACCEL_Y", dfXAccel);
	m_Notifications.push_back(XAccel);
	CMOOSMsg YAccel(MOOS_NOTIFY, "INS_ACCEL_X", dfYAccel);
	m_Notifications.push_back(YAccel);
	CMOOSMsg ZAccel(MOOS_NOTIFY, "INS_ACCEL_Z", dfZAccel);
	m_Notifications.push_back(ZAccel);
		

	//magnetic field data
	short nXMag = (Reply[19]<<8) + Reply[20];
        short nYMag = (Reply[21]<<8) + Reply[22];
        short nZMag = (Reply[23]<<8) + Reply[24];

	double dfGaussX = nXMag * 1.25 * 1.5/pow(2.0,15.0);
	double dfGaussY = nYMag * 1.25 * 1.5/pow(2.0,15.0);
	double dfGaussZ = nZMag * 1.25 * 1.5/pow(2.0,15.0);

	CMOOSMsg GaussX(MOOS_NOTIFY, "INS_MAG_Y", dfGaussX);
	m_Notifications.push_back(GaussX);
	CMOOSMsg GaussY(MOOS_NOTIFY, "INS_MAG_X", dfGaussY);
	m_Notifications.push_back(GaussY);
	CMOOSMsg GaussZ(MOOS_NOTIFY, "INS_MAG_Z", dfGaussZ);
	m_Notifications.push_back(GaussZ);

        
        if(m_pPort->IsVerbose())
        {
            //this allows us to print data in verbose mode
            //when teh port couldn't as it is verbose
            MOOSTrace("Roll = %7.3f Pitch = %7.3f Yaw = %7.3f Heading = %7.3f Temp = %3.2f\n",
		      df_deg_TRoll,
		      df_deg_TPitch,
		      dfYaw*180/PI,
		      dfHeading,
		      m_dfTemp);
        }
        
    }
    else
    {
	MOOSTrace("read %d byte while expecting %d\n",
		  nRead,
		  CROSSBOW_POLLED_ANGLE_MODE_REPLY_LENGTH);
    }
    
    return true;
    
}

bool CXbowDriver::GetStatus()
{
    return true;
}

bool CXbowDriver::SendMessage(CMOOSMsg Msg)
{
    return true;
}

bool CXbowDriver::Initialise()
{  

    double dfTwist = 0.0;
    if(!m_pMissionFileReader->GetConfigurationParam("TWIST", dfTwist))
    {
	if(m_pPort->IsVerbose())
	{
	    MOOSTrace("XbowDriver::Initialise(): COULD NOT READ TWIST: %d\n", dfTwist);
	}

        m_dfVehicleYToINSX = 0.0;
    }
    else
    {
	m_dfVehicleYToINSX = dfTwist;

	if(m_pPort->IsVerbose())
	{
	    MOOSTrace("XbowDriver::Initialise(): INS is TWISTed: %f\n", m_dfVehicleYToINSX);
	}
    }
    //need to know when to how big the returned bundle is
    m_pPort->SetIsCompleteReplyCallBack(XbowPortReadCallBack);
	
    //set to polled moade in angles...
    m_pPort->Write("P",1);	    
    MOOSPause(100);
    
    //set to angle mode
    m_pPort->Write("a",1);	 
    
    char Spare[10];
    m_pPort->ReadNWithTimeOut(Spare,1);
    
    if(Spare[0]!='A')
    {
        MOOSTrace("Unexpected reply when setting angle mode (expecting 'A')\n");
        return false;
    }
    
    
    return true;
}

bool CXbowDriver::INSPortReadCallBack(char *pData, int nBufferLen, int nRead)
{
    return nRead == CROSSBOW_POLLED_ANGLE_MODE_REPLY_LENGTH;
}
