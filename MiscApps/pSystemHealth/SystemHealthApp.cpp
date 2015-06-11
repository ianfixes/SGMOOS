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
//SystemHealthApp.cpp - implementation of the CSystemHealth class
//
//////////////////////////////////////////////////////////////////////

#include "SystemHealthApp.h"
#include <string.h>
#include <dirent.h>
#include <malloc.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace std;


/**
 * fixme
 */
CSystemHealthApp::CSystemHealthApp() : CMOOSApp()
{

}

CSystemHealthApp::~CSystemHealthApp()
{

}


/**
 * Camera Initialization
 * 
 * Make sure all config params are available, print an error if not
 * load all member variables 
 * init the camera driver
 */
bool CSystemHealthApp::OnStartUp()
{
    struct dirent **namelist;
    int i, n;

    //get hostname
    m_sHostname = GetFirstLine("/proc/sys/kernel/hostname");
    MOOSToUpper(m_sHostname);
    
    m_iTemperatureDirs = 0;

    //get initial cputime entries
    ReadCPU(m_iLastCPU);

    //get temperature entries
    n = scandir("/proc/acpi/thermal_zone", &namelist, 0, alphasort);
    if (n < 0)
    {
        MOOSTrace("Error reading /proc/acpi/thermal_zone!");
    }
    else 
    {
        m_iTemperatureDirs = (n - 2); //disregard . and .. 
        m_sTemperatureDir = new string[m_iTemperatureDirs];
        i = 0;

        while(n--) 
        {
            if ('.' != namelist[n]->d_name[0])
            {
                m_sTemperatureDir[i++] = namelist[n]->d_name;
                MOOSTrace(string("Added a thermal zone directory: ")
                          + namelist[n]->d_name + "\n"); 
            }
            free(namelist[n]);
        }
        free(namelist);
    }

    return true;
}



bool CSystemHealthApp::Iterate()
{
    m_dTimestamp = MOOSTime();
    GetTemperatures();
    GetCPU();
//    GetMemory();

    return true;
}

void CSystemHealthApp::GetTemperatures()
{
    string z;
    string tmp;
    string r;
    float f;

    for (int i = 0; i < m_iTemperatureDirs; ++i)
    {
        z = "/proc/acpi/thermal_zone/" + m_sTemperatureDir[i] + "/temperature";
        MOOSTrace("Getting Temperature in " + z + ": ");
        tmp = GetFirstLine(z);

        r = MOOSChomp(tmp, "       ");
        r = MOOSChomp(tmp, " C");

        f = atof(r.c_str());
        MOOSTrace(r + "\n");

        //log it
        HostNotify(string("TEMPERATURE_") + m_sTemperatureDir[i], f);
    }
}

//generate key based on hostname, use timestamp
void CSystemHealthApp::HostNotify(string key, double val)
{
    m_Comms.Notify(BuildHostKey(key), val, m_dTimestamp);
    return;
}

/*
//generate key based on hostname, use timestamp
void CSystemHealthApp::HostNotify(string key, string val)
{
    m_Comms.Notify(BuildHostKey(key), val, m_dTimestamp);
    return;
}
*/

string CSystemHealthApp::BuildHostKey(string key)
{
    return m_sHostname + "_" + key;
}


//read 
void CSystemHealthApp::ReadCPU(long* cputime)
{
    string s, r;

    s = GetFirstLine("/proc/stat");
    r = MOOSChomp(s, "cpu  ");
    for (int i = 0; i < 4; ++i)
    {
        cputime[i] = atol(MOOSChomp(s, " ").c_str());
    }
}

void CSystemHealthApp::GetCPU()
{
    long  cpu[4];
    long  usage, total;
    float percent;
    int   i;

    //read cpu into temp array
    ReadCPU(cpu);

    //perform subtraction / math /etc
    //BASED ON http://ubuntuforums.org/showthread.php?t=148781
    usage = 0;
    for (i = 0; i < 3; ++i)
    {
        usage += cpu[i] - m_iLastCPU[i];
    }

    total = usage + (cpu[3] - m_iLastCPU[3]);
    percent = (usage * 100.0) / total;

    //print to screen
    char c[255];
    sprintf(c, "CPU usage: %f\n", percent);

    MOOSTrace(c);

    //log it
    HostNotify(string("CPU_USAGE"), percent);


    //load tmp aray into member array
    for (i = 0; i < 4; ++i)
    {
        m_iLastCPU[i] = cpu[i];
    }
}

/*
void CSystemHealthApp::GetMemory()
{
    //fixme
}
*/


/*
string CSystemHealthApp::GetFileContents(string theFile)
{
    FILE* fp;
    long  len;
    char* buf;
    
    fp = fopen(theFile.c_str(), "rb");
    fseek(fp, 0, SEEK_END);            //go to end
    len = ftell(fp);                   //get position at end (length)
    fseek(fp, 0, SEEK_SET);            //go to beginning
    buf = (char*)malloc(len);          //malloc buffer
    fread(buf, len, 1, fp);            //read into buffer
    fclose(fp);
    return string(buf);
}
*/

string CSystemHealthApp::GetFirstLine(string theFile)
{
    FILE* fp;
    char buf[255];

    fp = fopen(theFile.c_str(), "r");
    if (feof(fp))
    {
        return  "";
    }
    fscanf(fp, "%[^\n]", buf);
    fclose(fp);

    return string(buf);
}





/*
bool CSystemHealthApp::OnCommandMsg(CMOOSMsg CmdMsg)
{
    if(CmdMsg.IsSkewed(MOOSTime()))
    {
        MOOSTrace("Got Skewed Command");
        return false;
    }

    if (MOOSStrCmp(sPrefix, "SOMETHING I RECOGNIZE"))
    {
        m_dfPeriodS = -1;
        return true;
    }

    MOOSTrace("Unknown command: ");
    MOOSTrace(CmdMsg.m_sVal);

    return false;
}



bool CSystemHealthApp::OnNewMail(MOOSMSG_LIST &NewMail)
{

    return m_driver->HandleMail(NewMail);
}



*/
