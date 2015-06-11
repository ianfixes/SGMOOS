PROJECT(iRemoteSG)

#what files are needed?
SET(IREMOTE_SRCS
  KeyAction.cpp KeyAction.h
  SGRemote.cpp SGRemote.h 
  iRemoteMain.cpp
  ${RESOURCES})

# Add executable called pLogger
ADD_EXECUTABLE( iRemoteSG ${IREMOTE_SRCS})

# indicate how to link
TARGET_LINK_LIBRARIES(iRemoteSG ${MOOSLIBS} ${PLATFORM_LIBS} )
