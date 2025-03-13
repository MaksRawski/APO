cmake_minimum_required(VERSION 3.16)
project(QtQuickApp LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

add_executable(QtQuickApp main.cpp resources.qrc)
target_link_libraries(QtQuickApp PRIVATE Qt6::Quick)
