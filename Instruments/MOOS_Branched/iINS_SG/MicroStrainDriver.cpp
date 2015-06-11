// MicroStrainDriver.cpp: implementation of the m_pMissionFileReader class.
//
//////////////////////////////////////////////////////////////////////
#include <MOOSLIB/MOOSLib.h>
#include "MicroStrainDriver.h"
#include <math.h>

#define MAX_PACKET_LEN 31

//MicroStrain Commands
#define CMD_TEMP 0x07
#define CMD_STAB_EULER 0x0E
#define CMD_STAB_EULER_and_ACCEL_and_RATE_VEC 0x31
#define CMD_INSTANT_EULER 0x0D

#define GYRO_GAIN_SCALE 8500
#define ACCEL_GAIN_SCALE 7000

using namespace std;
const string sNames[9] = {"ROLL", "PITCH", "YAW", "ACCEL_X", "ACCEL_Y", "ACCEL_Z", "ROLL_VEL", "PITCH_VEL", "YAW_VEL"};

const double dfTwoToTheFifteenth = pow(2.0 ,15.0);

bool MicroStrainPortReadCallBack(char * pData, int nBufferLen,int nRead)
{
    return nRead==11;
}

CMicroStrainDriver::CMicroStrainDriver()
{

	//Scaling Factors


    // scales angles to degrees
    m_dfScalingFactor[0] = 180.0/dfTwoToTheFifteenth;

    // scales accelerations to m/s**2
    m_dfScalingFactor[1] = ACCEL_GAIN_SCALE/(dfTwoToTheFifteenth*1000.0);
    m_dfScalingFactor[1] *= 9.81;  // G's -> m/s**2

    // scalse angular velocities to radians/sec
    m_dfScalingFactor[2] = GYRO_GAIN_SCALE/(dfTwoToTheFifteenth*1000.0);

    // Packet Length Definitions... add more as needed
    m_PacketLengths[0x0D] = 11;  //Send Instantaneous Euler Angles
    m_PacketLengths[0x0E] = 11;  //Send Gyro-Stabilized Euler Angles
    m_PacketLengths[0x31] = 23;  //Send Gyro-Stabilized Euler Angles & Accel & Rate Vector

    m_bUseInstant = false;
    m_nAverages = 1;
}

CMicroStrainDriver::~CMicroStrainDriver()
{

}

bool CMicroStrainDriver::GetData()
{

    //here we actually access serial ports etc
    if(m_pPort->IsStreaming())
    {
        MOOSTrace("MicroStrain must not be streaming\n");
        return false;
    }
    
    if(m_bUseInstant)
        GetInstantaneousAngles();

    //GetEulerAccelAndRate();

    return true;
}
	
bool CMicroStrainDriver::GetEulerAccelAndRate()
{
	DOUBLE_VEC dfVals;
	if(DoIO(0x31, dfVals))
	{
		double dfHeading, dfYaw = dfVals[2];
		double dfRoll = dfVals[0];
		double dfPitch = dfVals[1];
        
		if(!m_bUseInstant)
			ProcessAngles(dfRoll, dfPitch, dfYaw, dfHeading, "INS");

		//Process angular rates & translational acclerations.
		for(int i=3; i<9; i++)
        {
            string sMsgName = MOOSFormat("INS_%s", sNames[i].c_str());
            CMOOSMsg NewMsg(MOOS_NOTIFY, sMsgName.c_str() ,dfVals[i] * m_dfScalingFactor[i/3]);
            m_Notifications.push_back(NewMsg);
        }
		return true;
	}
	
	return false;
}

bool CMicroStrainDriver::GetInstantaneousAngles()
{

    DOUBLE_VEC dfVals;
    double dfHeading, dfYaw, dfRoll, dfPitch = 0.0;

    if(m_nAverages==1)
    {
        if(!DoIO(0x0D, dfVals))
            return false;
        
        dfYaw = dfVals[2];
        dfRoll = dfVals[0];
        dfPitch = dfVals[1];
    }
    else
    {
        double dfYawTotal, dfRollTotal, dfPitchTotal = 0.0;
    
        for(int i = 0; i<m_nAverages; i++)
        {
            if(!DoIO(0x0D, dfVals))
                    return false;
            
            //Keep the first one as a wrapping reference
            if(i==0)
            {
                    dfYaw = dfVals[2];
                    dfRoll = dfVals[0];
                    dfPitch = dfVals[1];
            }
    
            dfYawTotal += UnwrapAngle(dfVals[2], dfYaw);
            dfRollTotal += UnwrapAngle(dfVals[0], dfRoll);
            dfPitchTotal += UnwrapAngle(dfVals[1], dfPitch);
    
        }
    
        dfYaw = dfYawTotal/(double)m_nAverages;
        dfRoll = dfRollTotal/(double)m_nAverages;
        dfPitch = dfPitchTotal/(double)m_nAverages;
    }

    ProcessAngles(dfRoll, dfPitch, dfYaw, dfHeading, "INS");
    return true;

}

double CMicroStrainDriver::UnwrapAngle(double dfVal, double dfRef)
{
	if(dfVal-dfRef > dfTwoToTheFifteenth)
	{
		dfVal -= 2.0*dfTwoToTheFifteenth;
		//MOOSTrace("Wrapped Down\n");
	}
	else if(dfVal-dfRef < -dfTwoToTheFifteenth)
	{
		dfVal += 2.0*dfTwoToTheFifteenth;
		//MOOSTrace("Wrapped Up\n");
	}
	return dfVal;
}


//Scale angles & convert to radians.  For Yaw, add offsets & apply angle wrapping as well.
void CMicroStrainDriver::ProcessAngles(double& dfRoll, double& dfPitch, double& dfYaw, double& dfHeading, string sPrefix)
{
	dfRoll = MOOSDeg2Rad(dfRoll * m_dfScalingFactor[0]);
	dfPitch = MOOSDeg2Rad(dfPitch * m_dfScalingFactor[0]);

	dfHeading = dfYaw* m_dfScalingFactor[0] + m_dfVehicleYToINSX + m_dfMagneticOffset;
	
	//Heading -> Yaw.  Angle wrap both.
	dfYaw = MOOS_ANGLE_WRAP(MOOSDeg2Rad(dfHeading));
	dfHeading=MOOSRad2Deg(dfYaw); // To take advantage of the wrapping

	dfYaw *= -1.0;

	//Now publish
	CMOOSMsg Heading(MOOS_NOTIFY, sPrefix+"_HEADING", dfHeading);
	m_Notifications.push_back(Heading);

	CMOOSMsg Yaw(MOOS_NOTIFY, sPrefix+"_YAW", dfYaw);
	m_Notifications.push_back(Yaw);	

	CMOOSMsg Roll(MOOS_NOTIFY, sPrefix+"_ROLL", dfRoll);
	m_Notifications.push_back(Roll);

	CMOOSMsg Pitch(MOOS_NOTIFY, sPrefix+"_PITCH", dfPitch);
	m_Notifications.push_back(Pitch);

}

bool CMicroStrainDriver::GetStatus()
{
	return true;
}

bool CMicroStrainDriver::SendMessage(CMOOSMsg Msg)
{
	return true;
}

bool CMicroStrainDriver::Initialise()
{  
    
    if(!m_pMissionFileReader->GetConfigurationParam("TWIST", m_dfVehicleYToINSX))
    {
        m_dfVehicleYToINSX = 0.;
    }

	m_pMissionFileReader->GetConfigurationParam("AVERAGES", m_nAverages);
	m_pMissionFileReader->GetConfigurationParam("USEINSTANTANEOUS", m_bUseInstant);

	//need to know when to how big the returned bundle is
    m_pPort->SetIsCompleteReplyCallBack(MicroStrainPortReadCallBack);
	
	//could do a diag test
    
    
    return true;
}

bool CMicroStrainDriver::INSPortReadCallBack(char *pData, int nBufferLen, int nRead)
{
	return nRead == m_PacketLengths[0x0D];
}

bool CMicroStrainDriver::DoIO(char cCmd, DOUBLE_VEC& Vals)
{
	//ask the microstrain for a reading
    m_pPort->Write(&cCmd,1);
    
	int n_ReplyLen = m_PacketLengths[cCmd];
    unsigned char Reply[MAX_PACKET_LEN];
    
    //note local call back invoked here to specify termination
    int nRead = m_pPort->ReadNWithTimeOut((char*)Reply,n_ReplyLen);

    if(nRead == m_PacketLengths[cCmd])
    {
        if(Reply[0]!=cCmd)
        {
            MOOSTrace("Unexpected Header in MicroStrain reply\n");
		//Flush the buffer, so as to not put the microstrain in a bad state.
		MOOSPause(100);
		m_pPort->Flush();
			return false;
        }
        
		Vals.clear();
		int n_Vals = (m_PacketLengths[cCmd]-5)/2;

        for(int i=0; i<n_Vals; i++)
        {
			//assuming we're little endian here.
			short nTemp = (Reply[2*i+1]<<8) + Reply[2*i+2];
            Vals.push_back((double)nTemp);
        }
	}
	else
	{
	    MOOSTrace("read %d byte while expecting %d\n", nRead, m_PacketLengths[cCmd]);
		return false;
	}

	return true;	
}
