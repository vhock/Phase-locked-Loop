cmake_minimum_required(VERSION 3.5)

project(PLLInterface_Cpp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

include("C:/vcpkg/scripts/buildsystems/vcpkg.cmake")#might be needed ?
find_package(libssh CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread regex date_time )
set(CMAKE_AUTORCC ON)
set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        rputility.h
        rputility.cpp
        resources.qrc
        knight_rider.bit
)
set(RESOURCE_FILES
  knight_rider.bit
 )

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PLLInterface_Cpp
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET PLLInterface_Cpp APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(PLLInterface_Cpp SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(PLLInterface_Cpp
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(PLLInterface_Cpp PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(PLLInterface_Cpp PRIVATE ssh)
target_link_libraries(PLLInterface_Cpp PRIVATE ${Boost_LIBRARIES})

set_target_properties(PLLInterface_Cpp PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    RESOURCE "${RESOURCE_FILES}"
    )

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PLLInterface_Cpp)
endif()
