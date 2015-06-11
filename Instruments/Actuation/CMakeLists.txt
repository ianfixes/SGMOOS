PROJECT(iMotor)

# next few lines are simply to insert icons into
# the console application on windows
IF(WIN32)
    SET(RESOURCES icon1.ico iMotor.rc)
ENDIF(WIN32)

#what files are needed?
SET(DRIVER_SRCS
 MOOSMotorDriver.cpp MOOSMotorDriver.h
 MOOSCopleyDriver.cpp MOOSCopleyDriver.h
 MOOSParkerMotorDriver.cpp MOOSParkerMotorDriver.h
 MOOSJRKerrMotorDriver.cpp MOOSJRKerrMotorDriver.h )

SET(IMOTOR_SRCS 
 MOOSMotor.cpp MOOSMotor.h
 iMotorMain.cpp 
 ${DRIVER_SRCS} ) 

SET(IACTUATIONSG_SRCS
 SGActuationMain.cpp
 SGActuation.cpp SGActuation.h
 ActuatorSet.cpp ActuatorSet.h
 ReefExplorerThrusters.cpp ReefExplorerThrusters.h 
 OdysseyIVThrusters.cpp OdysseyIVThrusters.h 
 MOOSMotorDriver.cpp MOOSMotorDriver.h
 MOOSJRKerrMotorDriver.cpp MOOSJRKerrMotorDriver.h )


# Add executable called iMotor
ADD_EXECUTABLE( iMotor ${IMOTOR_SRCS})
ADD_EXECUTABLE( iActuationSG ${IACTUATIONSG_SRCS})

# indicate how to link
TARGET_LINK_LIBRARIES(iMotor SG ${MOOSLIBS} ${PLATFORM_LIBS} )
TARGET_LINK_LIBRARIES(iActuationSG SG ${MOOSLIBS} ${PLATFORM_LIBS} )

