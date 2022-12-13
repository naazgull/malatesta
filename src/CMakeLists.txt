set(MALATESTA_INCLUDE_DIR
  ${PROJECT_SOURCE_DIR}/include
)

add_library(malatestad SHARED)
target_sources(malatestad
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/app.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inotify.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filters.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/exclusions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/watches.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/controls.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin.cpp
    ${MALATESTA_INCLUDE_DIR}/malatesta/app.h
    ${MALATESTA_INCLUDE_DIR}/malatesta/inotify.h
    ${MALATESTA_INCLUDE_DIR}/malatesta/malatesta.h
    ${MALATESTA_INCLUDE_DIR}/malatesta/filters.h
    ${MALATESTA_INCLUDE_DIR}/malatesta/exclusions.h
    ${MALATESTA_INCLUDE_DIR}/malatesta/watches.h
    ${MALATESTA_INCLUDE_DIR}/malatesta/controls.h
)
target_include_directories(malatestad
  PRIVATE
    ${MALATESTA_INCLUDE_DIR}
  INTERFACE
    ${MALATESTA_INCLUDE_DIR}
)
target_link_libraries(malatestad
  PUBLIC
    ${ZAPATA_LIBS}
)  
set_target_properties(malatestad
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS} -fPIC"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS} -shared"
    LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libmalatestad.so
)

add_executable(malatesta)
add_dependencies(malatesta
  malatestad
)
target_sources(malatesta
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
target_include_directories(malatesta
  PRIVATE
    $<TARGET_PROPERTY:malatestad,INTERFACE_INCLUDE_DIRECTORIES>
)
target_link_libraries(malatesta
  PRIVATE
    ${ZAPATA_LIBS}
    $<TARGET_PROPERTY:malatestad,LIBRARIES>  
)
set_target_properties(malatesta
  PROPERTIES
    COMPILE_FLAGS "${ZAPATA_COMPILE_FLAGS}"
    LINK_FLAGS "${ZAPATA_LINK_FLAGS}"
)

include(GNUInstallDirs)
install(TARGETS malatestad malatesta
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME
    DESTINATION ${CMAKE_INSTALL_EXECDIR}
)
install(DIRECTORY ${MALATESTA_INCLUDE_DIR}/malatesta
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

