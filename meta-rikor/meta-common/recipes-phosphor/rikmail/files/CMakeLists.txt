cmake_minimum_required(VERSION 3.8)

project(rikmail)

find_package (PkgConfig REQUIRED)
pkg_check_modules (GLIB_PKG glib-2.0)
pkg_check_modules (GIO_PKG gio-unix-2.0)

include_directories (${GLIB_PKG_INCLUDE_DIRS})
include_directories (${GIO_PKG_INCLUDE_DIRS})
link_directories (${GLIB_PKG_LIBRARY_DIRS})

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/inc")
include_directories("${PROJECT_SOURCE_DIR}/src")


#                    COMMAND ${CMAKE_COMMAND} -E chdir ${PROJECT_SOURCE_DIR}/src gdbus-codegen --generate-c-code rikmail-manager --interface-prefix xyz.ares.rikmail. ${PROJECT_SOURCE_DIR}/src/xyz.ares.rikmail.xml


ADD_CUSTOM_COMMAND( OUTPUT rikmail-manager.h rikmail-manager.c
                    COMMAND gdbus-codegen --generate-c-code rikmail-manager --interface-prefix xyz.ares.rikmail. ${PROJECT_SOURCE_DIR}/src/xyz.openbmc_project.ares.rikmail.xml
                    DEPENDS ${PROJECT_SOURCE_DIR}/src/xyz.openbmc_project.ares.rikmail.xml 
                    COMMENT "Generating GDBUS source code from XML" )

ADD_CUSTOM_TARGET( rikmail-manager DEPENDS rikmail-manager.h rikmail-manager.c 
                   COMMENT "Checking if re-generation is required" )

find_package(Threads)

add_executable(${PROJECT_NAME} 
	src/main.cpp
	${PROJECT_BINARY_DIR}/rikmail-manager.c
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  COMPILE_OPTIONS -Wpedantic -Wall -Wextra
)

add_dependencies( ${PROJECT_NAME} rikmail-manager )

target_link_libraries(${PROJECT_NAME}
	${CMAKE_THREAD_LIBS_INIT}
    ${GIO_PKG_LIBRARIES}
    ${GLIB_PKG_LIBRARIES}
	stdc++fs
)

# configure_file(${PROJECT_SOURCE_DIR}/src/conf.json /tmp/rikmail/conf.json COPYONLY)

install (TARGETS ${PROJECT_NAME}   RUNTIME DESTINATION bin)
install (FILES   ${PROJECT_SOURCE_DIR}/rikmail.service   DESTINATION /lib/systemd/system    COMPONENT init)
# install (FILES   ${PROJECT_SOURCE_DIR}/src/conf.json    DESTINATION /etc/rikmail    COMPONENT config)

# https://stackoverflow.com/questions/35765106/symbolic-links-cmake

# add_executable(rikmailtest src/test.cpp)

# set_target_properties(rikmailtest PROPERTIES
#   CXX_STANDARD 17
#   CXX_STANDARD_REQUIRED ON
#   COMPILE_OPTIONS -Wpedantic -Wall -Wextra
# )

# target_link_libraries(rikmailtest
# 	stdc++fs
# )

# install (TARGETS rikmailtest   RUNTIME DESTINATION bin)

