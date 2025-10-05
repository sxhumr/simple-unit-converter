# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\Converter_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\Converter_autogen.dir\\ParseCache.txt"
  "Converter_autogen"
  )
endif()
