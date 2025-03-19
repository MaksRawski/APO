cmake_minimum_required(VERSION 3.16)
project(APO LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets)

add_executable(APO
  src/main.cpp
  src/mainwindow.h
  src/mainwindow.cpp
  src/ImageProcessor.h
  src/ImageProcessor.cpp
)

target_link_libraries(APO PRIVATE Qt6::Gui Qt6::Widgets)
