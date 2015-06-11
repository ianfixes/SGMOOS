PROJECT(pHelmSG)

#what files are needed?
SET(PHELM_SRCS 
 KatrinaBoatActuation.cpp KatrinaBoatActuation.h
 MOOSActuationMap.cpp MOOSActuationMap.h
 OdysseyIVActuation.cpp OdysseyIVActuation.h
 ReefExplorerActuation.cpp ReefExplorerActuation.h
 SGHelm.cpp SGHelm.h 
 SGHelmMain.cpp)

INCLUDE_DIRECTORIES(../SeaGrantTaskLib)

# Add executable called pLogger
ADD_EXECUTABLE( pHelmSG ${PHELM_SRCS})

# indicate how to link
TARGET_LINK_LIBRARIES(pHelmSG SeaGrantTask SG ${MOOSLIBS} ${PLATFORM_LIBS})
