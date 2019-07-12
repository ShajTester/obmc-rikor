cmake_minimum_required(VERSION 3.2)

project(rikcgi)

include_directories("${PROJECT_BINARY_DIR}")
# include_directories("${PROJECT_SOURCE_DIR}/include")

# set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

add_executable(${PROJECT_NAME}-login login.cpp)

set_target_properties(${PROJECT_NAME}-login PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  COMPILE_OPTIONS -Wpedantic -Wall -Wextra
)

target_link_libraries(${PROJECT_NAME}-login
	rikor-fru
)


add_executable(${PROJECT_NAME}-net net.cpp)

set_target_properties(${PROJECT_NAME}-net PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  COMPILE_OPTIONS -Wpedantic -Wall -Wextra
)

target_link_libraries(${PROJECT_NAME}-net
	rikor-fru
)


# add_library (rikor-fru SHARED IMPORTED)


install (TARGETS ${PROJECT_NAME}-login RUNTIME DESTINATION bin)
install (TARGETS ${PROJECT_NAME}-net RUNTIME DESTINATION bin)

# install (FILES ${PROJECT_BINARY_DIR}/dscan.conf DESTINATION /etc/dscan COMPONENT config)

