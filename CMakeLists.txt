PROJECT(SEAGRANT)
SET(SGLIB_PATH ${SEAGRANT_SOURCE_DIR}/SGLib)
INCLUDE_DIRECTORIES(${SGLIB_PATH})

#####################################################
# some definitions are always needed...
IF(WIN32)
    ADD_DEFINITIONS(-DWINDOWS_NT -D_CRT_SECURE_NO_DEPRECATE)
ENDIF(WIN32)
IF(UNIX)
    ADD_DEFINITIONS(-DUNIX)
ENDIF(UNIX)


#####################################################
# some platform libs are always required...
IF(WIN32)
  SET(PLATFORM_LIBS wsock32 comctl32)
ENDIF(WIN32)
IF(UNIX)
  SET(PLATFORM_LIBS m pthread)

  IF("${CMAKE_SYSTEM_NAME}" MATCHES "SunOS")
    SET(PLATFORM_LIBS ${PLATFORM_LIBS} socket nsl rt)
  ENDIF("${CMAKE_SYSTEM_NAME}" MATCHES "SunOS") 

ENDIF(UNIX)


#name the MOOSLIBS
SET(MOOSLIBS MOOS MOOSGen)


SET(SEAGRANT_DIRS SGLib Instruments Control MiscApps SwigBindings)

SET(BUILD_SG_GUI OFF CACHE BOOL "Build GUI: OdysseyIV c++ panel, rex panel, etc")
IF(BUILD_SG_GUI)
    SET(SEAGRANT_DIRS ${SEAGRANT_DIRS} GUI)
ENDIF(BUILD_SG_GUI)

SUBDIRS(${SEAGRANT_DIRS})

