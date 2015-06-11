PROJECT(MOOS_CV)

SUBDIRS(kht)
#ADD_DEFINITIONS(-DCHEVRON_HACK)

#what files are needed?
SET(MOOS_CV_SRC 
  RecordableVisionProcessor.cpp RecordableVisionProcessor.h
  HoughPipeFollow.cpp HoughPipeFollow.h
  HoughPipeFollow2.cpp HoughPipeFollow2.h
  PhaseCorrNav.cpp PhaseCorrNav.h
  PhaseCorrRotScale.cpp PhaseCorrRotScale.h
  OpticalFlow.cpp OpticalFlow.h
  LaserRangeFinder.cpp LaserRangeFinder.h
  MOOSVision.cpp MOOSVision.h
  iVisionMain.cpp
)
# Add executable called iDAQ
ADD_EXECUTABLE( iVision ${MOOS_CV_SRC})

SET (OPENCV_LIBS cv highgui)
SET (FFMPEG_LIBS avformat avcodec avutil swscale)
SET (FFMPEG_SUPPORT_LIBS bz2)
SET (FFTW_LIBS fftw3)
# indicate how to link
TARGET_LINK_LIBRARIES(iVision ${MOOSLIBS} ${PLATFORM_LIBS} ${OPENCV_LIBS} ${FFMPEG_LIBS} ${FFMPEG_SUPPORT_LIBS} ${FFTW_LIBS} KHT)
