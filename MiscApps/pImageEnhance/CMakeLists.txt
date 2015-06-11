PROJECT(PIMAGEENHANCE)


#what files are needed?
SET(PIMAGEENHANCE_SRCS 
  pImageEnhanceMain.cpp
  ImageEnhance.h
  ImageEnhance.cpp

)
# Add executable called pImageEnhance
ADD_EXECUTABLE( pImageEnhance ${PIMAGEENHANCE_SRCS})

FIND_PACKAGE(ImageMagick COMPONENTS Magick++)
INCLUDE_DIRECTORIES(${ImageMagick_Magick++_INCLUDE_DIR})


# indicate how to link
TARGET_LINK_LIBRARIES(pImageEnhance Magick++ MagickWand MagickCore SG ${MOOSLIBS} ${PLATFORM_LIBS})
