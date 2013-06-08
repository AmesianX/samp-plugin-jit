# This file is included only if sampgdk is built as a root project (i.e. not
# as a subdirectory).

list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/cmake/Modules"
)

if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang" OR
   "${CMAKE_C_COMPILER_ID}" STREQUAL "Intel")
  set(CMAKE_COMPILER_IS_GNUCC TRUE)
endif()
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR
   "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  set(CMAKE_COMPILER_IS_GNUCXX TRUE)
endif()

if(CMAKE_COMPILERIS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(flags "-m32 -Wno-attributes")
  if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flags}")
  endif()
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flags}")
  endif()
endif()
