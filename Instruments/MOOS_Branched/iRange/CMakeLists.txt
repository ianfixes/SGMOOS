#PROJECT(iRange)

#build a list of files to compile
SET(RANGE_SRCS 
iRangeMain.cpp
PSA916Driver.cpp
PSA916Driver.h
PSA916ForwardDriver.cpp
PSA916ForwardDriver.h
RangeInstrument.cpp
RangeInstrument.h
Tritech200Driver.cpp
Tritech200Driver.h
Tritech200ForwardDriver.cpp
Tritech200ForwardDriver.h)

# Add executable
ADD_EXECUTABLE( iRange ${RANGE_SRCS})

# Link the executable
TARGET_LINK_LIBRARIES(iRange SG ${MOOSLIBS} ${PLATFORM_LIBS} )

