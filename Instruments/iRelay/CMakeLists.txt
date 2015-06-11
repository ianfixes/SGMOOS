PROJECT(iRelay)


#what files are needed?
SET(RELAY_SRCS
MOOSRelay.cpp MOOSRelay.h
iRelayMain.cpp
)

# Add executable called MOOSDB
ADD_EXECUTABLE( iRelay  ${RELAY_SRCS})

TARGET_LINK_LIBRARIES( iRelay ${MOOSLIBS} ${PLATFORM_LIBS} )
