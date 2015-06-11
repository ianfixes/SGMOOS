PROJECT(SGBATT)

# next few lines are simply to insert icons into
# the console application on windows
IF(WIN32)
    SET(RESOURCES icon1.ico iDAQ.rc)
ENDIF(WIN32)

#what files are needed?
SET(SGBATT_SRCS 
  SeaGrantBattery.cpp SeaGrantBattery.h
  iBatterySGMain.cpp
)
# Add executable called iDAQ
ADD_EXECUTABLE( iBatterySG ${SGBATT_SRCS})

# indicate how to link
TARGET_LINK_LIBRARIES(iBatterySG SG ${MOOSLIBS} ${PLATFORM_LIBS} )
