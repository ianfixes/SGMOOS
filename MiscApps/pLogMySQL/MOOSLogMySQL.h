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

#if !defined(CMOOS_Log_MySQL)
#define CMOOS_Log_MySQL

#include <string>

//MySQL includes
#include <mysql++.h>
//#include <result.h>
//#include <connection.h>
//#include <datetime.h>

using namespace std;


/**
 * FIXME: needs class description
 */
class CMOOSLogMySQL : public CMOOSApp  
{
public:
    CMOOSLogMySQL();
    virtual ~CMOOSLogMySQL();

    bool Iterate();
    bool OnConnectToServer();
    bool OnNewMail(MOOSMSG_LIST &NewMail);
    bool OnStartUp();

protected:
    void PostMessage(std::string sMsg);
    bool CopyMissionFile();
    bool ConfigureLogging();
    bool OnLoggerRestart();
    bool OpenDBConnection();
    bool CloseDBConnection();
    bool OnNewMission(std::string sCommand);
    bool OnCommandMsg(CMOOSMsg Msg);
    bool LogAppDataAndMessages(MOOSMSG_LIST &NewMail);
    bool InsertConfigFileIntoTable(std::string sFilename);

    void ChangeSubscriptions(bool logMission);
    void BulkSubscribe(STRING_LIST* varlist);
    void BulkUnsubscribe(STRING_LIST* varlist);

    mysqlpp::SimpleResult DoInsertQuery(mysqlpp::Query& query);

    string SQLify(std::string S);

    double m_bLogging;
    double m_dfMissionStartTime;
    double m_bPostStatus;

    //what variables to log at what times
    STRING_LIST m_slLogParamsMission;
    STRING_LIST m_slLogParamsIdle;
    bool m_bLastLogWasMission;

    // DB info
    string m_sMySQLDatabase, m_sMySQLUser, m_sMySQLPassword, m_sMySQLHost;
    unsigned int m_nMySQLPort;
    bool m_bMySQLCompressData;
    mysqlpp::Connection m_MySQLConn;

    // Mission Data
    mysqlpp::ulonglong m_nCurrentMissionID;
    double m_dfCurrentMissionStartTime;
    string m_sDefaultLabel, m_sLocation, m_sVehicleName, m_sHoofFileName;
    
    typedef std::map< std::string, double > VARIABLE_TIMER_MAP;
    VARIABLE_TIMER_MAP m_MonitorMap;


};


#endif // !defined(CMOOS_Log_MySQL)
