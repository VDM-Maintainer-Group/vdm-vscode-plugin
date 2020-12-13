cmake_minimum_required(VERSION 3.12) #for FindPython feature
##
project(vdm-vscode-plugin VERSION 0.1)
set(CMAKE_CXX_STANDARD 14)
# set(CMAKE_CXX_STANDARD_REQUIRED True)
set(INSTALL_DIRECTORY "vdm/capability/inotify_hook")
## Find the library
find_package(PkgConfig REQUIRED)


## build kernel module of inotify capability
add_subdirectory("inotify-hook")


## build userspace daemon of inotify capability
# build the executable daemon
# file(GLOB DAEMON_API "./*.h")
add_executable(vdm-inotify-daemon daemon.c)
INSTALL(TARGETS vdm-inotify-daemon DESTINATION bin)
# build the shared dynamic library
add_library(vdm-inotify-hook SHARED daemon.c)
set_target_properties(vdm-inotify-hook PROPERTIES
                    PUBLIC_HEADER "daemon.h;")
INSTALL(TARGETS vdm-inotify-hook
        LIBRARY DESTINATION ${INSTALL_DIRECTORY}
        PUBLIC_HEADER DESTINATION ${INSTALL_DIRECTORY})


## build userspace application
add_executable(vscode_client vscode_client.c)
