cmake_minimum_required(VERSION 3.12) #for FindPython feature
##
project(vdm-vscode-plugin VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# find_package(PkgConfig REQUIRED)
# /usr/local/lib/cmake/vdm/capability/VDMInotifyLookupConfig.cmake
find_package(VDMInotifyLookup CONFIG REQUIRED)


# build userspace application
add_library(vscode-plugin SHARED vscode_plugin.c)
target_include_directories(vscode-plugin PUBLIC ${VDMInotifyLookup_INCLUDE_DIR})
target_link_libraries(vscode-plugin PUBLIC ${VDMInotifyLookup_LIBRARY})

add_executable(vscode-plugin-bin vscode_plugin.c)
target_link_libraries(vscode-plugin-bin PUBLIC ${VDMInotifyLookup_LIBRARY})
# INSTALL(TARGETS vscode-plugin-bin DESTINATION bin)
