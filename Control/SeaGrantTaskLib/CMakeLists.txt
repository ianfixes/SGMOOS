PROJECT(SGTaskLib)

SET(SGTASKLIB_SRCS
MOOSBehaviour.cpp MOOSBehaviour.h
MOOSMission.cpp MOOSMission.h
MOOSTaskReader.cpp MOOSTaskReader.h
PID.cpp PID.h
PathAction.cpp PathAction.h
MaintainDepth.cpp MaintainDepth.h
ConstantHeadingTask.cpp ConstantHeadingTask.h
EndMission.cpp EndMission.h
FaceXYPoint.cpp FaceXYPoint.h
FunctionGenerator.cpp FunctionGenerator.h
GoForward.cpp GoForward.h
GoToDepth.cpp GoToDepth.h
GoToAltitude.cpp GoToAltitude.h
GoToWayPoint.cpp GoToWayPoint.h
HoldingPattern.cpp HoldingPattern.h
#LimitAltitude.cpp LimitAltitude.h
#LimitBox.cpp LimitBox.h
#LimitDepth.cpp LimitDepth.h
MaintainAltitude.cpp MaintainAltitude.h
MaintainHeading.cpp MaintainHeading.h
MaintainPosition.cpp MaintainPosition.h
ManualControl.cpp ManualControl.h
SingleAction.cpp SingleAction.h
SupervisoryXY.cpp SupervisoryXY.h
#OrbitTask.cpp OrbitTask.h
#OverallTimeOut.cpp OverallTimeOut.h
#SteerThenDriveXYPatternTask.cpp SteerThenDriveXYPatternTask.h
SurveyTask.cpp SurveyTask.h
#ThirdPartyTask.cpp ThirdPartyTask.h
TimerTask.cpp TimerTask.h
TrackLineTask.cpp TrackLineTask.h
VerticalProfile.cpp VerticalProfile.h
XYPatternTask.cpp XYPatternTask.h
#ZPatternTask.cpp ZPatternTask.h
SafetyLimits.cpp SafetyLimits.h
RTUMonitor.cpp RTUMonitor.h
CameraTask.cpp CameraTask.h
FaceVerticalPipe.cpp FaceVerticalPipe.h
MaintainPipeDistance.cpp MaintainPipeDistance.h
MaintainPipeRelHeading.cpp MaintainPipeRelHeading.h
)

ADD_LIBRARY(SeaGrantTask ${SGTASKLIB_SRCS})
