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

#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include <MOOSLIB/MOOSApp.h>
#include <time.h>
#include <math.h>
#include "MOOSLogMySQL.h"
#include <MOOSGenLib/MOOSGenLibGlobalHelper.h>

#ifndef _WIN32
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include <ssqls.h>

using namespace mysqlpp;


//the sql_create_5 function is really a macro that defines
//    the structure of a database table.  then we get the syntax
//    of the insert statements automatically as well as a few other things
sql_create_5(app_data, 3, 5,
             sql_smallint_unsigned, mission_id,
             sql_double, elapsed_time,
             sql_varchar, varname,
             sql_varchar, app,
             sql_double, value);

sql_create_5(app_messages, 3, 5,
             sql_smallint_unsigned, mission_id,
             sql_double, elapsed_time,
             sql_varchar, varname,
             sql_varchar, app,
             sql_varchar, message);


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMOOSLogMySQL::CMOOSLogMySQL()
{
    SetAppFreq(5);
    
    m_sMySQLHost = "localhost";
    m_sMySQLDatabase = "moos_database";
    m_sMySQLUser = "mooslogger";
    m_sMySQLPassword = "moosloggerpasswd";
    
    m_bMySQLCompressData = false;  // you may want this set to 'true' if host isn't local
    m_nMySQLPort = 0;  // "0" is code for "use default MySQL port".
    
    m_nCurrentMissionID = 0;
    m_bLogging = false;
    m_sDefaultLabel = "";
    m_bPostStatus = true;
    m_bLastLogWasMission = false;
}


CMOOSLogMySQL::~CMOOSLogMySQL()
{
    CloseDBConnection();
}

bool CMOOSLogMySQL::OpenDBConnection()
{
    return m_MySQLConn.connect(m_sMySQLDatabase.c_str(),
        m_sMySQLHost.c_str(),
        m_sMySQLUser.c_str(),
        m_sMySQLPassword.c_str());
//        m_nMySQLPort,
//        m_bMySQLCompressData,
//        5);
}

bool CMOOSLogMySQL::CloseDBConnection()
{
    m_MySQLConn.disconnect();
    return true;
}

bool CMOOSLogMySQL::OnStartUp()
{

    // these all have sensible defaults, no need to fail if not present.
    m_MissionReader.GetConfigurationParam("MYSQLHOST", m_sMySQLHost);
    m_MissionReader.GetConfigurationParam("MYSQLDATABASE", m_sMySQLDatabase);
    m_MissionReader.GetConfigurationParam("MYSQLUSER", m_sMySQLUser);
    m_MissionReader.GetConfigurationParam("MYSQLPASSWORD", m_sMySQLPassword);

    if (!m_MissionReader.GetConfigurationParam("VehicleName", m_sVehicleName))
        m_MissionReader.GetValue("VehicleType", m_sVehicleName);

    m_MissionReader.GetConfigurationParam("Location", m_sLocation);

    // Only read new label if one doesn't exist.
    m_MissionReader.GetConfigurationParam("DefaultLabel", m_sDefaultLabel);
    
    // read in and set up all the names we are required to log..
    // note ConfigureLogging will return true even if registration
    // for variables doesn't complete (i.e DB not connected)
    
    if (!ConfigureLogging())
        return false;
    
    if (!OpenDBConnection())
    {
        MOOSTrace("Failed To Open MySQL Database Connection\n");
        return false;
    }
    return true;
}


bool CMOOSLogMySQL::OnConnectToServer()
{
    //ok so now lets register our interest in all these MOOS vars!
    
    //additional variables that are intersting to us..
    m_Comms.Register("MYSQL_CMD",0.5);
    m_Comms.Register("MISSION_FILE",0.5);

    return true;
}

bool CMOOSLogMySQL::OnNewMail(MOOSMSG_LIST &NewMail)
{

	//these three calls look through the incoming mail
	//and handle all appropriate logging
	
	LogAppDataAndMessages(NewMail);

	CMOOSMsg Msg;
	if (m_Comms.PeekMail(NewMail,"MYSQL_CMD",Msg))
	{
            if (!Msg.IsSkewed(MOOSTime()))
            {

//                std::string sCommand = Msg.GetString();
//                if (!OnNewCommand(sCommand))
//                    MOOSTrace("Command Failed (%s)\n", sCommand.c_str());

                if (!OnCommandMsg(Msg))
                    PostMessage((std::string)"Command Failed: " +  Msg.GetString());
                
            }
            else
                PostMessage((std::string)"Skewed Command\n");
	}
	if (m_Comms.PeekMail(NewMail,"MISSION_FILE",Msg))
	{
            if (!Msg.IsSkewed(MOOSTime()))
            {
                m_sHoofFileName = Msg.m_sVal;
            }
	}
	return true;
}

void CMOOSLogMySQL::PostMessage(std::string sMsg)
{
    MOOSTrace(sMsg + (std::string)"\n");
    m_Comms.Notify("MYSQL_MSG", sMsg);
}


bool CMOOSLogMySQL::OnCommandMsg(CMOOSMsg Msg)
{
    std::string sFullCommandString = Msg.GetString();
    
    if (MOOSStrCmp(sFullCommandString, "STARTIDLE"))
    {
        ChangeSubscriptions(false);
        if (OnNewMission("[Idle]"))
        {
            PostMessage(MOOSFormat("Started logging [Idle]"));
            m_bLogging = true;
            return true;
        }
        return false;        
    }

    //other commands may require a comma and an arg
    std::string sCommand = MOOSChomp(sFullCommandString, ",");
    
    if (MOOSStrCmp(sCommand, "START") || MOOSStrCmp(sCommand, "RESTART"))
    {
        std::string NewLogLabel;
        
        NewLogLabel = sFullCommandString;

        ChangeSubscriptions(true);
        
        if (OnNewMission(NewLogLabel))
        {
            PostMessage(MOOSFormat("Started logging %s", NewLogLabel.c_str()));
            m_bLogging = true;
            return true;
        }
        return false;
        
    }
    else if (MOOSStrCmp(sCommand, "STOP"))
    {
        m_bLogging = false;
        m_nCurrentMissionID = 0;
        PostMessage("Logging Stopped");
        
        return true;
    }
    return false;
    
}



bool CMOOSLogMySQL::ConfigureLogging()
{
    //figure out what we are required to log....
    //here we read in what we want to log from the mission file..
    STRING_LIST Params;
    if (m_MissionReader.GetConfiguration(m_sAppName, Params))
    {
        //this will make columns in sync log in order they
        //were declared in *.moos file
        Params.reverse();
        
        STRING_LIST::iterator p;
        for (p = Params.begin(); p != Params.end(); p++)
        {
            std::string sParam = *p;
            std::string sWhat = MOOSChomp(sParam, "=");
            std::string sVar;
            
            if (MOOSStrCmp(sWhat, "LOG"))
            {
                sVar = MOOSChomp(sParam, "@");
                m_slLogParamsMission.push_back(sVar);
            }

            if (MOOSStrCmp(sWhat, "LOGIDLE"))
            {
                sVar = MOOSChomp(sParam, "@");
                m_slLogParamsIdle.push_back(sVar);
            }
        }

        BulkSubscribe(&m_slLogParamsIdle);
    }
    
    return true;
    
}

void CMOOSLogMySQL::ChangeSubscriptions(bool logMission)
{
    if (logMission && !m_bLastLogWasMission)
    {
        BulkUnsubscribe(&m_slLogParamsIdle);
        BulkSubscribe(&m_slLogParamsMission);
    }
    else if (!logMission && m_bLastLogWasMission)
    {
        BulkUnsubscribe(&m_slLogParamsMission);
        BulkSubscribe(&m_slLogParamsIdle);
    }
}

void CMOOSLogMySQL::BulkSubscribe(STRING_LIST* varlist)
{
    double dfPeriod = 0.1;
    
    std::string sVar;
    STRING_LIST::iterator v;
    for (v = varlist->begin(); v != varlist->end(); v++)
    {
        m_Comms.Register(sVar, dfPeriod);
    }
}

void CMOOSLogMySQL::BulkUnsubscribe(STRING_LIST* varlist)
{
    std::string sVar;
    STRING_LIST::iterator v;
    for (v = varlist->begin(); v != varlist->end(); v++)
    {
        m_Comms.UnRegister(sVar);
    }
}

bool CMOOSLogMySQL::Iterate()
{
    return true;
}


bool CMOOSLogMySQL::OnNewMission(std::string sNewLogLabel="")
{
    
    if (sNewLogLabel.empty())
        sNewLogLabel = m_sDefaultLabel;
    
    std::string sLat, sLon;
    if (!m_MissionReader.GetValue("LatOrigin",sLat))
        sLat = "NULL";
    
    if (!m_MissionReader.GetValue("LongOrigin",sLon))
        sLon = "NULL";
    
    //Create a new entry in the "mission" table
    if (!m_MySQLConn.connected())
        return false;
    
    m_dfCurrentMissionStartTime = MOOSTime();
    struct tm *Now;
    time_t aclock = (time_t) m_dfCurrentMissionStartTime;
    Now = localtime( &aclock );
    
    mysqlpp::Query query = m_MySQLConn.query();
    
    query   << "INSERT INTO mission "
            << "(date,time,label,vehicle_name,location,origin_latitude,origin_longitude) "
            << "VALUES (" 
            << MOOSFormat("\"%d-%d-%d\"", Now->tm_year+1900, Now->tm_mon+1, Now->tm_mday) << ","
            << MOOSFormat("\"%d:%d:%d\"", Now->tm_hour, Now->tm_min, Now->tm_sec) << ","
            << SQLify(sNewLogLabel) << ","
            << SQLify(m_sVehicleName) << ","
            << SQLify(m_sLocation) << ","
            << sLat << ","
            << sLon << ")";
    
    if (mysqlpp::SimpleResult res = DoInsertQuery(query))
    {
        m_nCurrentMissionID = res.insert_id();
        m_bLogging=true;
        
        // Log .moos file
        InsertConfigFileIntoTable(m_sMissionFile);
        
        // Log .hoof file;
        if (!m_sHoofFileName.empty())
            InsertConfigFileIntoTable(m_sHoofFileName);
        
        return true;
    }
    return false;
}

bool CMOOSLogMySQL::OnLoggerRestart()
{
    return OnNewMission();
}

bool CMOOSLogMySQL::LogAppDataAndMessages(MOOSMSG_LIST &NewMail)
{    
    bool bOK = true;
    
    //log asynchrounously...
    if (m_MySQLConn.connected() && m_bLogging)
    {
        
        vector<app_data> data_list;
        vector<app_messages> msg_list;
        
        for (MOOSMSG_LIST::iterator msg = NewMail.begin(); msg!=NewMail.end(); msg++)
        {
            
            if (msg->IsDouble())
            {
                double dfVal = msg->GetDouble();
                
                if (finite(dfVal))
                {
                    data_list.push_back(
                        app_data(m_nCurrentMissionID, 
                                 msg->GetTime() - m_dfCurrentMissionStartTime, 
                                 msg->GetKey(), 
                                 msg->GetSource(), 
                                 dfVal)
                        );
                }
            }
            else
            {
                msg_list.push_back(
                    app_messages(m_nCurrentMissionID, 
                                 msg->GetTime() - m_dfCurrentMissionStartTime, 
                                 msg->GetKey(), 
                                 msg->GetSource(), 
                                 msg->GetString())
                    );
//                query.insert(*msg_list.begin());
                
            }
        }
        
        if (!data_list.empty())
        {
            mysqlpp::Query data_query = m_MySQLConn.query();
            data_query.insert(data_list.begin(), data_list.end());
            if (!DoInsertQuery(data_query))
                bOK = false;

        }
        
        if (!msg_list.empty())
        {
            mysqlpp::Query msg_query = m_MySQLConn.query();
            msg_query.insert(msg_list.begin(), msg_list.end());
            if (!DoInsertQuery(msg_query))
                bOK = false;
        }
        
    }
    return bOK;
}


bool CMOOSLogMySQL::InsertConfigFileIntoTable(std::string sFilename)
{
    ifstream filestream(sFilename.c_str(), ios::in|ios::ate);
    if (!filestream)
    {
        MOOSTrace("Could not open file: (%s)\n", sFilename.c_str());
        return false;
    }
    size_t nSize = filestream.tellg();
    char* cBuffer = new char[nSize];
    filestream.seekg(0, ios::beg);
    filestream.read(cBuffer, nSize);
    filestream.close();
    
    std::string FileContents;
    FileContents.assign(cBuffer, nSize);
    delete cBuffer;
    
    mysqlpp::Query query = m_MySQLConn.query();
    query << "INSERT INTO text_files VALUES (" 
          << m_nCurrentMissionID << ","
          << SQLify(sFilename) << ",\"" 
          << FileContents << "\")";
    
    DoInsertQuery(query);
    
    return true;
}

mysqlpp::SimpleResult CMOOSLogMySQL::DoInsertQuery(mysqlpp::Query& query)
{
    //cerr << MOOSTime()-m_dfCurrentMissionStartTime << " " << query.preview() << endl;
    mysqlpp::SimpleResult res;
    try
    {
        res = query.execute();
    }
    catch (const mysqlpp::BadQuery& er)
    {
        // Handle any query errors
        PostMessage(MOOSFormat("Query error: %s", er.what()));
    }
    catch (const mysqlpp::Exception& er) {
        // Catch-all for any other MySQL++ exceptions
        PostMessage(MOOSFormat("Error: %s", er.what()));
    }
    return res;
}

// Sanitize std::strings for SQL.  Put quotes around if not "NULL".
std::string CMOOSLogMySQL::SQLify(std::string S)
{
    return (MOOSStrCmp(S, "NULL") || S.empty())
        ? S
        : (std::string)"\""+ S + (std::string)"\"";
}
