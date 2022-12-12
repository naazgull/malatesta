cmake_minimum_required(VERSION 3.18)

project(malatesta VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(THIRD_PARTY_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/third_party/install)

list(APPEND CMAKE_INSTALL_RPATH
  ${CMAKE_INSTALL_PREFIX}/bin
  ${CMAKE_INSTALL_PREFIX}/lib
  ${CMAKE_INSTALL_PREFIX}/lib64
)
list(APPEND CMAKE_BUILD_RPATH
  ${THIRD_PARTY_INSTALL_PREFIX}/bin
  ${THIRD_PARTY_INSTALL_PREFIX}/lib
  ${THIRD_PARTY_INSTALL_PREFIX}/lib64
)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
set(CMAKE_SKIP_INSTALL_RPATH OFF)
set(CMAKE_SKIP_BUILD_RPATH OFF)
set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)

message("-- Runtime path: ${CMAKE_INSTALL_RPATH}")
message("-- Build runtime path: ${CMAKE_BUILD_RPATH}")

set(ZAPATA_COMPILE_FLAGS "-O3 -DNDEBUG -fno-omit-frame-pointer -Wno-interference-size -Wall -pedantic -Wno-restrict")
set(ZAPATA_LINK_FLAGS "")

string(TOLOWER "${CMAKE_BUILD_TYPE}" ZAPATA_BUILD_TYPE)
if(ZAPATA_BUILD_TYPE STREQUAL debug)
  set(ZAPATA_COMPILE_FLAGS "-O0 -ggdb3 -fno-omit-frame-pointer -Wno-interference-size -Wall -pedantic -Wno-restrict")
endif()

message("-- Compiler flags: ${ZAPATA_COMPILE_FLAGS}")
message("-- Linker flags: ${ZAPATA_LINK_FLAGS}")

include(third_party/third_party.cmake)

include_directories(
  SYSTEM
    ${THIRD_PARTY_INSTALL_PREFIX}/include
)

add_subdirectory(src)

# ------------------------------------------------------------------------
# TARGET: uninstall
#
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()
