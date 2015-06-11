PROJECT(iMultisonde)

# next few lines are simply to insert icons into
# the console application on windows
IF(WIN32)
    SET(RESOURCES icon1.ico iMultisonde.rc)
ENDIF(WIN32)

#what files are needed?
SET(IMULTISONDE_SRCS 
  MOOSMultisonde.cpp MOOSMultisonde.h
  MOOSMultisondeDriver.cpp MOOSMultisondeDriver.h
  MOOSHydrolabDriver.cpp MOOSHydrolabDriver.h
  MOOSYSIDriver.cpp MOOSYSIDriver.h
  SeabirdCTDDriver.cpp SeabirdCTDDriver.h
  iMultisondeMain.cpp
)
# Add executable called iMultisonde
ADD_EXECUTABLE( iMultisonde ${IMULTISONDE_SRCS})

# indicate how to link
TARGET_LINK_LIBRARIES(iMultisonde SG ${MOOSLIBS} ${PLATFORM_LIBS} )
