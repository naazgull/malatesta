include(ExternalProject)
include(ProcessorCount)
processorcount(n)

find_library(ZAPATA_BASE_LIB NAMES zapata-base
  PATHS
    ${THIRD_PARTY_INSTALL_PREFIX}/lib
    ${CMAKE_INSTALL_PREFIX}/lib
    /usr/lib
    /usr/lib64
    /usr/local/lib
    /usr/local/lib64
)
  
add_library(zapata
  SHARED IMPORTED
)

if (NOT ZAPATA_BASE_LIB)
  message("-- Zapata library not found, downloading and compiling")
  
  set(ZAPATA_INSTALL_LIBDIR ${THIRD_PARTY_INSTALL_PREFIX}/lib)

  externalproject_add(
    zapata_download
    SOURCE_DIR "${PROJECT_SOURCE_DIR}/third_party/zapata"
    GIT_REPOSITORY "https://github.com/naazgull/zapata"
    GIT_TAG zapata-2.0.0-issue-115
    CMAKE_ARGS
      -Wno-dev
      -DCMAKE_INSTALL_PREFIX=${THIRD_PARTY_INSTALL_PREFIX}
      -DCMAKE_INSTALL_RPATH=${CMAKE_INSTALL_RPATH}
      -DCMAKE_BUILD_TYPE=Release
    BUILD_COMMAND
      ${CMAKE_COMMAND} --build <BINARY_DIR> -j${n}
  )
  add_dependencies(zapata
    zapata_download
  )
else()
  message("-- Found Zapata: ${ZAPATA_BASE_LIB}")
  get_filename_component(ZAPATA_INSTALL_LIBDIR ${ZAPATA_BASE_LIB} DIRECTORY)
endif()

set(ZAPATA_LIBS
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-base.so
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-common-catalogue.so
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-events.so
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-engine-events.so
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-engine-startup.so
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-engine-rest.so
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-globals.so
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-io-pipe.so
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-io-socket.so
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-io-stream.so
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-net-http.so
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-net-local-plugin.so
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-net-local.so
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-net-transport.so
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-parser-functional.so
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-parser-http.so
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-parser-json.so
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-parser-uri.so
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-storage-connector.so
  ${ZAPATA_INSTALL_LIBDIR}/libzapata-storage-sqlite.so
)

set_target_properties(zapata
  PROPERTIES
    IMPORTED_LOCATION ${ZAPATA_INSTALL_LIBDIR}/libzapata-base.so
)

include(GNUInstallDirs)
install(DIRECTORY ${THIRD_PARTY_INSTALL_PREFIX}/include
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY ${THIRD_PARTY_INSTALL_PREFIX}/lib
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  FILES_MATCHING PATTERN "*.so*"
)
install(DIRECTORY ${THIRD_PARTY_INSTALL_PREFIX}/lib
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  FILES_MATCHING PATTERN "*.a*"
)
install(DIRECTORY ${THIRD_PARTY_INSTALL_PREFIX}/lib64
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  FILES_MATCHING PATTERN "*.so*"
)
install(DIRECTORY ${THIRD_PARTY_INSTALL_PREFIX}/lib64
  DESTINATION ${CMAKE_INSTALL_PREFIX}
  FILES_MATCHING PATTERN "*.a*"
)
