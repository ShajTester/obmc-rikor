cmake_minimum_required(VERSION 3.8)

project(rikfan)

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/inc")
include_directories("${PROJECT_SOURCE_DIR}/src")

find_package(Threads)

add_executable(${PROJECT_NAME} src/main.cpp src/util.cpp src/ec/pid.cpp src/ec/stepwise.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  COMPILE_OPTIONS -Wpedantic -Wall -Wextra
)

target_link_libraries(${PROJECT_NAME}
	${CMAKE_THREAD_LIBS_INIT}
	stdc++fs
)

configure_file(${PROJECT_SOURCE_DIR}/src/conf.json /tmp/rikfan/conf.json COPYONLY)

install (TARGETS ${PROJECT_NAME}   RUNTIME DESTINATION bin)
install (FILES   ${PROJECT_SOURCE_DIR}/rikfan.service   DESTINATION /lib/systemd/system    COMPONENT init)
install (FILES   ${PROJECT_SOURCE_DIR}/src/conf.json    DESTINATION /etc/rikfan    COMPONENT config)

# https://stackoverflow.com/questions/35765106/symbolic-links-cmake

add_executable(rikfantest src/test.cpp)

set_target_properties(rikfantest PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  COMPILE_OPTIONS -Wpedantic -Wall -Wextra
)

target_link_libraries(rikfantest
	stdc++fs
)

# install (TARGETS rikfantest   RUNTIME DESTINATION bin)

