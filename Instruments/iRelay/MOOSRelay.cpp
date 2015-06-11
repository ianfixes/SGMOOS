
#include "MOOSRelay.h"
#include <MOOSGenLib/MOOSGenLib.h>
using namespace std;

CMOOSRelay::CMOOSRelay() : CMOOSInstrument() { }

CMOOSRelay::~CMOOSRelay() { }

bool CMOOSRelay::OnNewMail(MOOSMSG_LIST &NewMail)
{

    string sVarName = GetAppName() + "_ENABLE";
    MOOSToUpper(sVarName);
    CMOOSMsg Msg;
    
    if(m_Comms.PeekMail(NewMail, sVarName, Msg))
    {
        if(!Msg.IsSkewed( MOOSTime() ) )
        {
            bool m_bEnableVal = MOOSStrCmp(Msg.GetString(), "True");
            DoEnable(m_bEnableVal);
        }
    }
    return true; 
}

bool CMOOSRelay::OnConnectToServer()
{
    string sVarName = GetAppName() + "_ENABLE";
    MOOSToUpper(sVarName);
    return m_Comms.Register(sVarName, 0.2);
}

bool CMOOSRelay::Iterate()
{ 
    return true;
}

bool CMOOSRelay::OnStartUp()
{       
    CMOOSInstrument::OnStartUp();

    if(!SetupPort())
    {
        MOOSTrace("%s::OnStartUp - Failed to setup port\n", m_sAppName.c_str());
        return false;
    }

    bool bTemp;
    if(m_MissionReader.GetConfigurationParam("ENABLEONSTART",bTemp))
    {
        DoEnable(bTemp);
    }

    return true;
}

bool CMOOSRelay::DoEnable(bool bEnable)
{
    MOOSDebugWrite(MOOSFormat("(%s) Turning relay %s\n", m_sAppName.c_str(), bEnable ? "on" : "off"));
    m_Port.Write("ENABLE\r\n", 8);
}

