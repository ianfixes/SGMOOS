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
//
// SeaGrantBattery.cpp: implementation of the CSeaGrantBattery class.
//
//////////////////////////////////////////////////////////////////////

#include "SeaGrantBattery.h"
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSeaGrantBattery::CSeaGrantBattery()
{

    //some sensible defaults (missionfile can overwrite this)
    SetAppFreq(2);
    SetCommsFreq(5);

}

CSeaGrantBattery::~CSeaGrantBattery()
{
}

bool CSeaGrantBattery::OnStartUp()
{
    CMOOSInstrument::OnStartUp();

    if(!SetupPort())
    {
	MOOSTrace("%s::OnStartUp - Failed to setup port\n", m_sAppName.c_str());
        return false;
    }
    
    string sMyNameIs=m_sAppName.c_str();
    MOOSToUpper(sMyNameIs);

    AddMOOSVariable("Voltage",       "", sMyNameIs+"_VOLTAGE",      0.1);
    AddMOOSVariable("Voltages",      "", sMyNameIs+"_VOLTAGES",     0.1);
    AddMOOSVariable("Temperatures",  "", sMyNameIs+"_TEMPERATURES", 0.1);
    AddMOOSVariable("Current",       "", sMyNameIs+"_CURRENT",      0.1);
    AddMOOSVariable("MinCell",       "", sMyNameIs+"_MIN_CELL",     0.1);
    AddMOOSVariable("MaxCell",       "", sMyNameIs+"_MAX_CELL",     0.1);
    AddMOOSVariable("Temp",          "", sMyNameIs+"_TEMP",         0.1);
    AddMOOSVariable("Summary",       "", sMyNameIs+"_SUMMARY",      0.1);

    return RegisterMOOSVariables();
}

bool CSeaGrantBattery::Iterate()
{

    if(GetData())
        PublishFreshMOOSVariables();

    return true;
}

bool CSeaGrantBattery::GetData()
{
    string sReply;

    switch (GetIterateCount() % 3)
    {
    case 0:
        //battery summary
        m_Port.Write("#B\r\n", 4);
        if(m_Port.GetTelegram(sReply, 6.0))
        {
            ParseSummary(sReply);
        }
        else
        {
            SetMOOSVar("Summary", "No Response", MOOSTime());
        }
        
        break;
        
    case 1:
        //individual cell voltages
        m_Port.Write("#V\r\n", 4);
        if (m_Port.GetTelegram(sReply, 6.0))
        {
            ParseVoltages(sReply);
        }

        break;
        
    case 2:
        //individual cell temperatures
        m_Port.Write("#T\r\n", 4);
        if (m_Port.GetTelegram(sReply, 6.0))
        {
            ParseTemps(sReply);
        }
        break;

    default:
        break;
    }


    
    return true;
}

//$B 96.4V 1.7A 4.15V 4.18V 25C

bool CSeaGrantBattery::ParseSummary(string sSummary)
{

    double dfNow = MOOSTime();

    string sTemp = MOOSChomp(sSummary, " ");
    if(!MOOSStrCmp(sTemp, "$B"))
        return false;
    
    SetMOOSVar("Summary", sSummary, dfNow);

    sTemp = MOOSChomp(sSummary, "V ");
    SetMOOSVar("Voltage", atof(sTemp.c_str()), dfNow);

    sTemp = MOOSChomp(sSummary, "A ");
    SetMOOSVar("Current", atof(sTemp.c_str()), dfNow);

    sTemp = MOOSChomp(sSummary, "V ");
    SetMOOSVar("MinCell", atof(sTemp.c_str()), dfNow);

    sTemp = MOOSChomp(sSummary, "V ");
    SetMOOSVar("MaxCell", atof(sTemp.c_str()), dfNow);

    sTemp = MOOSChomp(sSummary, "C");
    SetMOOSVar("Temp", atof(sTemp.c_str()), dfNow);

    return true;
}

bool CSeaGrantBattery::ParseVoltages(string sVoltages)
{
    double v[24];
    double v_max, v_min, v_tot;
    string sTemp;

    double dfNow = MOOSTime();
    
    sTemp = MOOSChomp(sVoltages, " ");
    if (!MOOSStrCmp(sTemp, "$V"))
        return false;
    
    SetMOOSVar("Voltages", sVoltages, dfNow);


    for (int i = 0; i < 24; ++i)
    {
        sTemp = MOOSChomp(sVoltages, " ");
        v[i] = atof(sTemp.c_str());
    }
 
    v_tot = 0; 
    //find total, min and max voltages
    for (int i = 0; i < 24; ++i)
    {
        if (i == 0 || v_max < v[i]) 
            v_max = v[i];
        
        if (i == 0 || v_min > v[i]) 
            v_min = v[i];

        v_tot += v[i];
    }    

    SetMOOSVar("MinCell", v_min, dfNow);
    SetMOOSVar("MaxCell", v_max, dfNow);
    SetMOOSVar("Voltage", v_tot, dfNow);

    return true;
}

bool CSeaGrantBattery::ParseTemps(string sTemps)
{
    double t[24];
    double t_max;
    string sTemp;

    double dfNow = MOOSTime();

    sTemp = MOOSChomp(sTemps, " ");
    if (!MOOSStrCmp(sTemp, "$T"))
        return false;

    
    SetMOOSVar("Temperatures", sTemps, dfNow);

    for (int i = 0; i < 24; ++i)
    {
        sTemp = MOOSChomp(sTemps, " ");
        t[i] = atof(sTemp.c_str());
    }

    //find max temp
    for (int i = 0; i < 24; ++i)
    {
        if (i == 0 || t_max < t[i]) 
            t_max = t[i];
    }    

    SetMOOSVar("Temp", t_max, dfNow);

    return true;
}
