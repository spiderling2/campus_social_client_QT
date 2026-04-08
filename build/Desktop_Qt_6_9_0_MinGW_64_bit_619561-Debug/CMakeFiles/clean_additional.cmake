# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\CampusSocialClient_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\CampusSocialClient_autogen.dir\\ParseCache.txt"
  "CampusSocialClient_autogen"
  )
endif()
