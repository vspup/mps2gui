cmake_minimum_required(VERSION 3.5)

project(mps2gui VERSION 0.1 LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -no-pie")

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMake")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Charts Core Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Charts Core Gui)
find_package(nng REQUIRED)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/shim_window.cpp
        inc/shim_window.h
        src/variables_list.cpp
        inc/variables_list.h
        src/response_parser.cpp
        inc/response_parser.h
        inc/mainwindow.h
        src/mainwindow.ui
        lib/uart-escape.c
        inc/uart-escape.h
        lib/cwpack_utils.c
        inc/cwpack_utils.h
        lib/cwpack.c
        inc/cwpack.h
        inc/cwpack_config.h
        inc/cwpack_internals.h
        lib/electabuzz_common.c
        inc/electabuzz_common.h
        lib/electabuzz_client.c
        inc/electabuzz_client.h
        lib/electabuzz_client_helpers.c
        inc/electabuzz_client_helpers.h
        inc/commands.h
        src/chart.cpp
        inc/chart.h
        src/chartview.cpp
        inc/chartview.h
)

message(STATUS "PROJECT_SOURCES=" ${PROJECT_SOURCES})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(mps2gui
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET mps2gui APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(mps2gui SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(mps2gui
            ${PROJECT_SOURCES}
        )
    endif()
endif()

#target_link_libraries(mps2gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
#target_link_libraries(mps2gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets nng::nng)
#target_link_libraries(mps2gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${nng_LIBRARY}::nng)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(mps2gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Charts Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui)
else ()
    #target_link_libraries(mps2gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets ${nng_LIBRARY}::nng)
    target_link_libraries(mps2gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets nng::nng)
endif()

target_link_libraries(mps2gui PRIVATE ${nng_LIBRARIES})
file(COPY ${nng_LIBRARY} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(mps2gui PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS mps2gui
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(mps2gui)
endif()
