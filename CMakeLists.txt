cmake_minimum_required(VERSION 3.16)

project(Estadisticas)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Widgets Charts Concurrent REQUIRED)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(Estadisticas
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.ui
    include/mainwindow.h
)

target_link_libraries(Estadisticas PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Charts
    Qt6::Concurrent
)

target_include_directories(Estadisticas
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    include
)
