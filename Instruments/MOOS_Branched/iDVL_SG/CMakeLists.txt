PROJECT(iDVL_SG)

#what files are needed?
SET(IDVL_SG_SRCS 
 DVLInstrument.cpp DVLInstrument.h
 iDVLMain.cpp  
 ${RESOURCES})

# Add executable called iDVL_SG
ADD_EXECUTABLE( iDVL_SG ${IDVL_SG_SRCS})

# indicate how to link
TARGET_LINK_LIBRARIES(iDVL_SG ${MOOSLIBS} ${PLATFORM_LIBS} )
