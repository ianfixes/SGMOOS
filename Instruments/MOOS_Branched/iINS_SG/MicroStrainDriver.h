// MicroStrainDriver.h: interface for the CMicroStrainDriver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MICROSTRAINDRIVER_H__64B11E7B_9DE4_4C6F_A9BC_60964234EBFB__INCLUDED_)
#define AFX_MICROSTRAINDRIVER_H__64B11E7B_9DE4_4C6F_A9BC_60964234EBFB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "INSDriver.h"
typedef vector<double> DOUBLE_VEC;

class CMicroStrainDriver : public CINSDriver  
{
public:
	bool INSPortReadCallBack(char * pData, int nBufferLen,int nRead);
	CMicroStrainDriver();
	virtual ~CMicroStrainDriver();
	bool GetStatus();
	bool SendMessage(CMOOSMsg Msg);
	bool Initialise();
	bool GetData();

private:
	bool GetInstantaneousAngles();
	bool GetEulerAccelAndRate();
	double UnwrapAngle(double dfVal, double dfRef);

	void ProcessAngles(double& dfRoll, double& dfPitch, double& dfYaw, double& dfHeading, string sPrefix);
	bool DoIO(char cCmd, DOUBLE_VEC& Vals);

    char cPacketHeader;
    int nPacketLength;
    double m_dfScalingFactor[3];  // [angular, acceleration, angular_velocity]
	map<char, int> m_PacketLengths;

	bool m_bUseInstant;
	int m_nAverages;
};

#endif // !defined(AFX_MICROSTRAINDRIVER_H__64B11E7B_9DE4_4C6F_A9BC_60964234EBFB__INCLUDED_)
