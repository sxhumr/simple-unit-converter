cmake_minimum_required(VERSION 3.16)

project(Converter VERSION 1.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(SOURCES
    main.cpp
    mainwindow.cpp
    units.cpp
)

set(HEADERS
    mainwindow.h
    units.h
)

qt_add_executable(Converter
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(Converter PRIVATE Qt6::Widgets)

set_target_properties(Converter PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
