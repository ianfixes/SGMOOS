PROJECT(iINS_SG)

#what files are needed?
SET(IINS_SRCS 
 INSInstrument.cpp INSInstrument.h
 INSDriver.cpp INSDriver.h 
 MicroStrainDriver.cpp MicroStrainDriver.h
 XbowDriver.cpp XbowDriver.h
 iINSMain.cpp  
 ${RESOURCES})

# Add executable called iDepth
ADD_EXECUTABLE( iINS_SG ${IINS_SRCS})

# indicate how to link
TARGET_LINK_LIBRARIES(iINS_SG ${MOOSLIBS} ${PLATFORM_LIBS} )
