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
#include "MOOSMission.h"
#include "MOOSTaskReader.h"

CMOOSMission::CMOOSMission()
{
}

CMOOSMission::~CMOOSMission()
{
}

void CMOOSMission::Destroy()
{
    for(TASK_LIST::iterator i=m_Behaviors.begin(); i!=m_Behaviors.end(); i++)
        delete (*i);
}

bool CMOOSMission::ReadHoofFile(std::string sTaskFile, CProcessConfigReader* pMissionFileReader)
{
    CMOOSTaskReader reader;

    if(!reader.Run(sTaskFile.c_str(), pMissionFileReader, m_Behaviors))
        return false;

    m_sName=reader.GetName();
	m_sFileName = sTaskFile;

    return true;

}

bool CMOOSMission::GetRegistrations(STRING_LIST &List)
{
    for(TASK_LIST::iterator i=m_Behaviors.begin(); i!=m_Behaviors.end(); i++)
        (*i)->GetRegistrations(List);

    return true;
}

bool CMOOSMission::GetNotifications(MOOSMSG_LIST &List)
{   
    for(TASK_LIST::iterator i=m_Behaviors.begin(); i!=m_Behaviors.end(); i++)
        (*i)->GetNotifications(List);

    return true;
}

bool CMOOSMission::OnNewMail(MOOSMSG_LIST &List)
{    
    TASK_LIST::iterator p;

    for(p = m_Behaviors.begin();p!=m_Behaviors.end();p++)
    {
        CSGMOOSBehaviour* pBehaviour = *p;
        pBehaviour->OnNewMail(List);
    }
    return true;
}

bool CMOOSMission::Run(CPathAction& DesiredAction)
{       
    TASK_LIST::iterator p;
    bool bActionRequest = false;
    double dfTimeNow = MOOSTime();

    for(p = m_Behaviors.begin();p!=m_Behaviors.end();p++)
    {            
        CSGMOOSBehaviour* pBehaviour = *p;

        pBehaviour->SetTime(dfTimeNow);

        if(pBehaviour->ShouldRun())
        {
            bActionRequest |= pBehaviour->Run(DesiredAction);
        }
    }

    //Post-Iteration Step: Process all the flags generated;
    if((*m_Behaviors.begin())->HasFlags())
    {
        for(p = m_Behaviors.begin();p!=m_Behaviors.end();p++)
            (*p)->ProcessFlags();
        
        (*m_Behaviors.begin())->ClearFlags();
    }
    return bActionRequest;
}

string CMOOSMission::GetChart()
{
    if(GenerateChart())
        return m_sChart;
    else
        return "Invalid Chart";
}

bool CMOOSMission::GenerateChart()
{
    string sNode, sEdge;
    TASK_LIST::iterator p;
    for(p = m_Behaviors.begin(); p!=m_Behaviors.end(); p++)
    {
         sNode+=(*p)->GetChartNodeEntry();
         sEdge+=(*p)->GetChartEdgeEntry();
    }

    m_sChart = "digraph " + m_sName + "\n{\n";
//    m_sChart += "graph[rankdir=LR]";

    m_sChart += sNode;
    m_sChart += sEdge;
    m_sChart += "}\n";


    return true;
}
