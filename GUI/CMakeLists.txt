PROJECT(uControlPanel)

#build a list of files to compile
SET(uCP_SRCS
wxMOOSApp.cpp wxMOOSApp.h
MOOSPanel.cpp MOOSPanel.h
SGHelmPanel.cpp SGHelmPanel.h
VarLogPanel.cpp VarLogPanel.h
RemoteControlPanel.cpp RemoteControlPanel.h
SummaryPanel.cpp SummaryPanel.h
NavSummaryPanel.cpp NavSummaryPanel.h
YSISummaryPanel.cpp YSISummaryPanel.h
BatterySummaryPanel.cpp BatterySummaryPanel.h
ActuationPanel.cpp ActuationPanel.h
SupervisoryPanel.cpp SupervisoryPanel.h
MissionSummaryPanel.cpp MissionSummaryPanel.h
MOOSPokePanel.cpp MOOSPokePanel.h
wxMOOSLib.h
)

SET(wxWidgets_USE_LIBS base core)

# Wx stuff
FIND_PACKAGE(wxWidgets)

IF(wxWidgets_FOUND)
  INCLUDE(${wxWidgets_USE_FILE})
ELSE(wxWidgets_FOUND)
  MESSAGE(SEND_ERROR "wxWidgets not found")
ENDIF(wxWidgets_FOUND)

ADD_DEFINITIONS(${wxWidgets_DEFINITIONS})

# Add executable
ADD_EXECUTABLE( uRexPanel uRexPanel.cpp ${uCP_SRCS})
TARGET_LINK_LIBRARIES( uRexPanel ${MOOSLIBS} ${PLATFORM_LIBS} ${wxWidgets_LIBRARIES} )

# Add executable
ADD_EXECUTABLE( uOdysseyIVPanel uOdysseyIVPanel.cpp ${uCP_SRCS})
TARGET_LINK_LIBRARIES( uOdysseyIVPanel ${MOOSLIBS} ${PLATFORM_LIBS} ${wxWidgets_LIBRARIES} )
