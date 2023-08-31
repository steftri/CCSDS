cmake_minimum_required(VERSION 3.14)

project(CCSDS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ask for all warnings and strict compliance with the standard
set(CMAKE_CXX_FLAGS "-Wall -Werror -pedantic")

add_library(CCSDS STATIC
  src/ccsds.h
  src/configCCSDS.h
  src/ccsds_clcw.h
  src/ccsds_clcw.cpp
  src/ccsds_cltu.h
  src/ccsds_cltu.cpp
  src/ccsds_transferframe.h
  src/ccsds_transferframe.cpp
  src/ccsds_transferframe_tc.h
  src/ccsds_transferframe_tm.h
  src/ccsds_transferframe_tc.cpp
  src/ccsds_transferframe_tm.cpp
  src/ccsds_spacepacket.h
  src/ccsds_spacepacket.cpp
  src/pus_tc.h
  src/pus_tc.cpp
)

target_compile_definitions(CCSDS PRIVATE CCSDS_LIBRARY)
target_include_directories(CCSDS PUBLIC src)
