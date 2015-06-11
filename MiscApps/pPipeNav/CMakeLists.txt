PROJECT(PPIPENAVAPP)

#what files are needed?
SET(PPIPENAVAPP_SRCS 
  pPipeNavMain.cpp
  PipeNavApp.h
  PipeNavApp.cpp

)
# Add executable called iDAQ
ADD_EXECUTABLE( pPipeNavApp ${PPIPENAVAPP_SRCS})

# indicate how to link
TARGET_LINK_LIBRARIES(pPipeNavApp ${MOOSLIBS} ${PLATFORM_LIBS})
