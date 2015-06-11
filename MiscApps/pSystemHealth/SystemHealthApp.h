/*************************************************************************

    SGMOOS: A set of libraries, scripts & applications which extend MOOS 
    (Mission Oriented Operating Suite by Dr. Paul Newman) for use in 
    Autonomous Underwater Vehicles & Autonomous Surface Vessels. 

    Copyright (C) 2006,2007,2008,2009 Massachusetts Institute of 
    Technology Sea Grant

    This software was written by Ian Katz & others at MIT Sea Grant.
    contact: ijk5@mit.edu

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
//SystemHealthApp.h - interface for the CSystemHealth class
//
/////////////////////////////////////


#if !defined(AFX_SYSTEMHEALTHAPP_H_INCLUDED_)
#define AFX_SYSTEMHEALTHAPP_H_INCLUDED_

#include <MOOSLIB/MOOSLib.h>
#include <string>


using namespace std;

class CSystemHealthApp : public CMOOSApp
{
public:
    CSystemHealthApp();
    virtual ~CSystemHealthApp();
    

    /**
    *we override the OnNewMail to handle Task's sending of variable
    *with more than one value 
    */
/*
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool OnCommandMsg(CMOOSMsg CmdMsg);
*/

    bool OnStartUp();
    bool Iterate();

protected:

    string* m_sTemperatureDir;
    int     m_iTemperatureDirs;
    long    m_iLastCPU[4];
    double  m_dTimestamp;
    string  m_sHostname;

    string GetFileContents(string theFile);
    string GetFirstLine(string theFile);

    void ReadCPU(long* cpu_time);

    void HostNotify(string key, string val);
    void HostNotify(string key, double val);
    string BuildHostKey(string key);

    void GetTemperatures();
    void GetCPU();
    void GetMemory();

    float ParseTemperature(string acpiTemp);
    float ParseCpu(string cpuFirstLine);
    float ParseMemory(string mem);

};

#endif // !defined(AFX_SYSTEMHEALTHAPP_H_INCLUDED_)

