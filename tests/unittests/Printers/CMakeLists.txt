# FIXME: had to add this
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-permissive -pedantic -Wall -Wextra")

include_directories(
    ${CMAKE_SOURCE_DIR}/modules/Ubuntu/Components/Extras/Printers
)

find_package(Qt5Gui REQUIRED)
find_package(Qt5Test REQUIRED)

set(MOCK_SOURCES
    mockbackend.h
)

add_executable(testPrintersPrinter tst_printer.cpp ${MOCK_SOURCES})
target_link_libraries(testPrintersPrinter UbuntuSettingsPrintersQml Qt5::Test Qt5::Gui)
add_test(tst_printer testPrintersPrinter)

add_executable(testPrintersPrinterJob tst_printerjob.cpp ${MOCK_SOURCES})
target_link_libraries(testPrintersPrinterJob UbuntuSettingsPrintersQml Qt5::Test Qt5::Gui)
add_test(tst_printerjob testPrintersPrinterJob)

add_executable(testPrintersPrinters tst_printers.cpp ${MOCK_SOURCES})
target_link_libraries(testPrintersPrinters UbuntuSettingsPrintersQml Qt5::Test Qt5::Gui)
add_test(tst_printers testPrintersPrinters)

add_executable(testPrintersPrinterModel tst_printermodel.cpp ${MOCK_SOURCES})
target_link_libraries(testPrintersPrinterModel UbuntuSettingsPrintersQml Qt5::Test Qt5::Gui)
add_test(tst_printermodel testPrintersPrinterModel)

add_executable(testPrintersPrinterFilter tst_printerfilter.cpp ${MOCK_SOURCES})
target_link_libraries(testPrintersPrinterFilter UbuntuSettingsPrintersQml Qt5::Test Qt5::Gui)
add_test(tst_printerfilter testPrintersPrinterFilter)

add_executable(testPrintersDriverModel tst_drivermodel.cpp ${MOCK_SOURCES})
target_link_libraries(testPrintersDriverModel UbuntuSettingsPrintersQml Qt5::Test Qt5::Gui)
add_test(tst_drivermodel testPrintersDriverModel)

add_executable(testPrintersJobModel tst_jobmodel.cpp ${MOCK_SOURCES})
target_link_libraries(testPrintersJobModel UbuntuSettingsPrintersQml Qt5::Test Qt5::Gui)
add_test(tst_jobmodel testPrintersJobModel)

add_executable(testPrintersJobFilter tst_jobfilter.cpp ${MOCK_SOURCES})
target_link_libraries(testPrintersJobFilter UbuntuSettingsPrintersQml Qt5::Test Qt5::Gui)
add_test(tst_jobfilter testPrintersJobFilter)

add_executable(testPrintersSignalHandler tst_signalhandler.cpp ${MOCK_SOURCES})
target_link_libraries(testPrintersSignalHandler UbuntuSettingsPrintersQml Qt5::Test Qt5::Gui)
add_test(tst_signalhandler testPrintersSignalHandler)
