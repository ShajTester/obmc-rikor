cmake_minimum_required(VERSION 3.8)

project(rikbtnd)

find_package (PkgConfig REQUIRED)
pkg_check_modules (GLIB_PKG glib-2.0)
pkg_check_modules (GIO_PKG gio-unix-2.0)

include_directories (${GLIB_PKG_INCLUDE_DIRS})
include_directories (${GIO_PKG_INCLUDE_DIRS})
link_directories (${GLIB_PKG_LIBRARY_DIRS})

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/inc")
include_directories("${PROJECT_SOURCE_DIR}")


#                    COMMAND ${CMAKE_COMMAND} -E chdir ${PROJECT_SOURCE_DIR}/src gdbus-codegen --generate-c-code ${PROJECT_NAME}-manager --interface-prefix xyz.ares.${PROJECT_NAME}. ${PROJECT_SOURCE_DIR}/src/xyz.ares.rikfan.xml


ADD_CUSTOM_COMMAND( OUTPUT ${PROJECT_NAME}-manager.h ${PROJECT_NAME}-manager.c
                    COMMAND gdbus-codegen --generate-c-code ${PROJECT_NAME}-manager --interface-prefix xyz.ares.${PROJECT_NAME}. ${PROJECT_SOURCE_DIR}/xyz.openbmc_project.ares.${PROJECT_NAME}.xml
                    DEPENDS ${PROJECT_SOURCE_DIR}/xyz.openbmc_project.ares.${PROJECT_NAME}.xml 
                    COMMENT "Generating GDBUS source code from XML" )

ADD_CUSTOM_TARGET( ${PROJECT_NAME}-manager DEPENDS ${PROJECT_NAME}-manager.h ${PROJECT_NAME}-manager.c 
                   COMMENT "Checking if re-generation is required" )

find_package(Threads)

add_executable(${PROJECT_NAME} 
	rikbtnd.c
	${PROJECT_BINARY_DIR}/${PROJECT_NAME}-manager.c
	)

set_target_properties(${PROJECT_NAME} PROPERTIES
  COMPILE_OPTIONS -Wpedantic -Wall -Wextra
)

add_dependencies( ${PROJECT_NAME} ${PROJECT_NAME}-manager )

target_link_libraries(${PROJECT_NAME}
	${CMAKE_THREAD_LIBS_INIT}
    ${GIO_PKG_LIBRARIES}
    ${GLIB_PKG_LIBRARIES}
    gpio
)

# configure_file(${PROJECT_SOURCE_DIR}/src/conf.json /tmp/${PROJECT_NAME}/conf.json COPYONLY)

install (TARGETS  ${PROJECT_NAME}   RUNTIME DESTINATION bin)
install (FILES    ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.service   DESTINATION /lib/systemd/system    COMPONENT init)
install (PROGRAMS ${PROJECT_SOURCE_DIR}/rikbtnd-setup.sh          DESTINATION bin)
install (PROGRAMS ${PROJECT_SOURCE_DIR}/rikbtnd-afterpoweron.sh   DESTINATION bin)
install (FILES    ${PROJECT_SOURCE_DIR}/.profile   DESTINATION /home/root    COMPONENT init)
# install (FILES   ${PROJECT_SOURCE_DIR}/src/conf.json    DESTINATION /etc/${PROJECT_NAME}    COMPONENT config)

# https://stackoverflow.com/questions/35765106/symbolic-links-cmake

# add_executable(rikfantest src/test.cpp)

# set_target_properties(rikfantest PROPERTIES
#   CXX_STANDARD 17
#   CXX_STANDARD_REQUIRED ON
#   COMPILE_OPTIONS -Wpedantic -Wall -Wextra
# )

# target_link_libraries(rikfantest
# 	stdc++fs
# )

# install (TARGETS rikfantest   RUNTIME DESTINATION bin)

