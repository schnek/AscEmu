# Copyright (c) 2014-2021 AscEmu Team <http://www.ascemu.org>

set(PATH_PREFIX Objects)

set(SRC_OBJECTS_FILES
   ${PATH_PREFIX}/CObjectFactory.cpp
   ${PATH_PREFIX}/CObjectFactory.h
   ${PATH_PREFIX}/DynamicObject.cpp
   ${PATH_PREFIX}/DynamicObject.h
   ${PATH_PREFIX}/Faction.cpp
   ${PATH_PREFIX}/Faction.h
   ${PATH_PREFIX}/GameObject.cpp
   ${PATH_PREFIX}/GameObject.h
   ${PATH_PREFIX}/MovementDefines.h
   ${PATH_PREFIX}/MovementInfo.h
   ${PATH_PREFIX}/Object.cpp
   ${PATH_PREFIX}/Object.h
   ${PATH_PREFIX}/ObjectMgr.cpp
   ${PATH_PREFIX}/ObjectMgr.h
   ${PATH_PREFIX}/Transporter.cpp
   ${PATH_PREFIX}/Transporter.h
   ${PATH_PREFIX}/G3DPosition.hpp
   
   # MIT
   ${PATH_PREFIX}/ObjectDefines.h
)

source_group(Objects FILES ${SRC_OBJECTS_FILES})
unset(PATH_PREFIX)
