# requires minimum cmake version
cmake_minimum_required(VERSION 3.5)

# produce the cmake var PROJECT_NAME
project(assignment_motor-control)

# mandatory use of these packages
find_package(YARP 3.1.101 REQUIRED)
find_package(ICUBcontrib REQUIRED)

# extend the current search path used by cmake to load helpers
list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})

# helpers defining certain macros
include(ICUBcontribHelpers)
include(ICUBcontribOptions)

# specify to install in $ICUBcontrib_DIR/bin
icubcontrib_set_default_prefix()

# building server
add_executable(server ${CMAKE_SOURCE_DIR}/src/server.cpp)
target_compile_definitions(server PRIVATE _USE_MATH_DEFINES)
target_link_libraries(server ${YARP_LIBRARIES})
install(TARGETS server DESTINATION bin)

# building client
add_executable(client ${CMAKE_SOURCE_DIR/}src/client.cpp)
target_compile_definitions(client PRIVATE _USE_MATH_DEFINES)
target_link_libraries(client ${YARP_LIBRARIES})
install(TARGETS client DESTINATION bin)

# building trigger
add_executable(trigger ${CMAKE_SOURCE_DIR/}src/trigger.cpp)
target_compile_definitions(trigger PRIVATE _USE_MATH_DEFINES)
target_link_libraries(trigger ${YARP_LIBRARIES})
install(TARGETS trigger DESTINATION bin)

# generate ad-hoc project to perform "make uninstall"
icubcontrib_add_uninstall_target()

file(GLOB scripts ${CMAKE_SOURCE_DIR}/app/scripts/*.xml)
yarp_install(FILES ${scripts} DESTINATION ${ICUBCONTRIB_APPLICATIONS_INSTALL_DIR})
