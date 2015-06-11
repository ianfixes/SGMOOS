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
#ifndef MOOSTASKLIBH
#define MOOSTASKLIBH

#ifdef _WIN32
	#pragma warning(disable : 4786)
#endif


//export the following classes.....

//add utility classes here
#include "MOOSTaskReader.h"
#include "PathAction.h"
#include "PID.h"

//add new task definition here
#include "MOOSBehaviour.h"
#include "MOOSMission.h"
#include "MaintainHeading.h"
#include "MaintainDepth.h"
#include "MaintainAltitude.h"
#include "MaintainPosition.h"
#include "FaceXYPoint.h"
#include "GoForward.h"
#include "VerticalProfile.h"
#include "GoToWayPoint.h"
#include "EndMission.h"
#include "TimerTask.h"
#include "HoldingPattern.h"
#include "ConstantHeadingTask.h"
#include "GoToDepth.h"
#include "GoToAltitude.h"
//#include "ZPatternTask.h"
#include "XYPatternTask.h"
//#include "ThirdPartyTask.h"
//#include "OrbitTask.h"
#include "TrackLineTask.h"
#include "SurveyTask.h"
#include "ManualControl.h"
#include "FunctionGenerator.h"
#include "SupervisoryXY.h"
#include "SafetyLimits.h"
#include "CameraTask.h"
#include "SingleAction.h"
#include "RTUMonitor.h"
#include "FaceVerticalPipe.h"
#include "MaintainPipeDistance.h"
#include "MaintainPipeRelHeading.h"

#endif


