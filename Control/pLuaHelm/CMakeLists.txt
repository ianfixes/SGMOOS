PROJECT(PLUAHELM)

#what files are needed?
SET(PLUAHELM_SRCS 
  pLuaHelmMain.cpp
  LuaHelmApp.h
  LuaHelmApp.cpp
  StateMachine.h
  HelmConfig.h
  HelmConfig.cpp
  LuaMission.h
  LuaMission.cpp
  MOOSLuaEnvironment.h
  MOOSLuaEnvironment.cpp
  
  MOOSActuationMap.h
  MOOSActuationMap.cpp
  OdysseyIVActuation.h
  OdysseyIVActuation.cpp
  KatrinaBoatActuation.h
  KatrinaBoatActuation.cpp
  ReefExplorerActuation.h
  ReefExplorerActuation.cpp
)

INCLUDE_DIRECTORIES(../SeaGrantTaskLib)

# Add executable called pLuaHelm
ADD_EXECUTABLE( pLuaHelm ${PLUAHELM_SRCS})

# indicate how to link
TARGET_LINK_LIBRARIES(pLuaHelm SG SeaGrantTask ${MOOSLIBS} lua5.1 ${PLATFORM_LIBS})
