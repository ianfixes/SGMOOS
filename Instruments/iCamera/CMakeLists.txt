PROJECT(ICAMERA)

# next few lines are simply to insert icons into
# the console application on windows
IF(WIN32)
    SET(RESOURCES icon1.ico iDAQ.rc)
ENDIF(WIN32)

#what files are needed?
  #BaslerCameraDriver.cpp
  #BaslerCameraDriver.h
SET(ICAMERA_SRCS 
  CameraDriverDC1394.cpp
  CameraDriverDC1394.h
  CameraDriverDummy.cpp
  CameraDriverDummy.h
  CameraDriverGphoto2.h
  CameraDriverGphoto2.cpp
  CLIBuster.h
  CameraDriver.cpp
  CameraDriver.h
  CameraApp.cpp
  CameraApp.h
  iCameraMain.cpp
  conversions.cpp
  conversions.h

)
# Add executable called iDAQ
ADD_EXECUTABLE( iCamera ${ICAMERA_SRCS})

# indicate how to link
TARGET_LINK_LIBRARIES(iCamera SG ${MOOSLIBS} ${PLATFORM_LIBS} dc1394_control raw1394 expect)
