project(ubuntu-ui-extras)
cmake_minimum_required(VERSION 2.8.9)

# Standard install paths
include(GNUInstallDirs)

# enable QML debugging
if(CMAKE_BUILD_TYPE MATCHES DEBUG OR CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DQT_QML_DEBUG)
endif()

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(Qt5Test)

add_definitions(-DQT_NO_KEYWORDS)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# make non compiled files visible in qtcreator
file(GLOB_RECURSE NON_COMPILED_FILES *.qml *.js *.py *.svg *.png *.in *.json *.desktop *.pot qmldir)
add_custom_target(NON_COMPILED_TARGET ALL SOURCES ${NON_COMPILED_FILES})

# for dh_translations to extract the domain
# (regarding syntax consistency, see http://pad.lv/1181187)
set (GETTEXT_PACKAGE "ubuntu-ui-extras")

add_subdirectory(modules)

enable_testing()

add_subdirectory(tests)
add_subdirectory(po)
