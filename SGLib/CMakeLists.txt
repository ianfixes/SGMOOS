#PROJECT(SGLib)
SET(SGLIB_SRCS 
SensorDataFilter.cpp
MedianFilter.cpp
AverageFilter.cpp
MOOSInstrumentDriver.cpp
MOOSInstrumentFamily.cpp
SeaGrantGlobalHelper.cpp
SensorDataFilter.h
MedianFilter.h
AverageFilter.h
MOOSInstrumentDriver.h
MOOSInstrumentFamily.h
SeaGrantGlobalHelper.h
)

ADD_LIBRARY(SG ${SGLIB_SRCS})
