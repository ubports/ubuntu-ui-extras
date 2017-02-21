project(UbuntuComponentsExtrasPrintersQml)

add_definitions(-DUBUNTUSETTINGSPRINTERS_LIBRARY)

# FIXME: had to add this
include(FindCups)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CUPS_INCLUDE_DIR})

# FIXME: had to add this
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-permissive -pedantic -Wall -Wextra")

find_package(Qt5Gui REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Concurrent REQUIRED)

# FIXME: had to add this
if(NOT CUPS_FOUND)
message(FATAL_ERROR "Could not find cups.")
endif()

qt5_add_dbus_interface(
    GEN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/org.cups.cupsd.Notifier.xml
    cupsdnotifier)

add_library(UbuntuComponentsExtrasPrintersQml SHARED
    ${GEN_SOURCES}
    backend/backend.cpp
    backend/backend_cups.cpp
    backend/backend_pdf.cpp

    cups/ippclient.cpp
    cups/printerdriverloader.cpp
    cups/printerloader.cpp

    models/drivermodel.cpp
    models/jobmodel.cpp
    models/printermodel.cpp

    printer/printer.cpp
    printer/printerjob.cpp
    printer/printersignalhandler.cpp
    printers/printers.cpp

    enums.h
    i18n.cpp
    plugin.cpp
    structs.h
    utils.h
)

target_link_libraries(UbuntuComponentsExtrasPrintersQml
    Qt5::DBus
    Qt5::Gui
    Qt5::PrintSupport
    Qt5::Qml
    Qt5::Concurrent
    ${CUPS_LIBRARIES}
)

# FIXME: had to add this
find_package(QmlPlugins)

macro(add_usc_plugin PLUGIN VERSION PATH)
    export_qmlfiles(${PLUGIN} ${PATH} DESTINATION ${QT_IMPORTS_DIR} ${ARGN})
    export_qmlplugin(${PLUGIN} ${VERSION} ${PATH} DESTINATION ${QT_IMPORTS_DIR} ${ARGN})
endmacro()

add_usc_plugin(Ubuntu.Components.Extras.Printers 0.1 Ubuntu/Components/Extras/Printers TARGETS UbuntuComponentsExtrasPrintersQml)
