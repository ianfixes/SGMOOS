PROJECT(PSYSTEMHEALTH)

# next few lines are simply to insert icons into
# the console application on windows
IF(WIN32)
    SET(RESOURCES icon1.ico iDAQ.rc)
ENDIF(WIN32)

#what files are needed?
SET(PSYSTEMHEALTH_SRCS 
  pSystemHealthMain.cpp
  SystemHealthApp.h
  SystemHealthApp.cpp

)
# Add executable called iDAQ
ADD_EXECUTABLE( pSystemHealth ${PSYSTEMHEALTH_SRCS})

# indicate how to link
TARGET_LINK_LIBRARIES(pSystemHealth SG ${MOOSLIBS} ${PLATFORM_LIBS})
