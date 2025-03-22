cmake_minimum_required(VERSION 3.16)
project(APO LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets)

add_executable(APO
  src/main.cpp

  # UI
  src/mainwindow.hpp               src/mainwindow.cpp
  src/mdiChild.hpp                 src/mdiChild.cpp
  src/imageLabel.hpp               src/imageLabel.cpp

  # Processing
  src/ImageProcessor.hpp           src/ImageProcessor.cpp
)

target_link_libraries(APO PRIVATE Qt6::Gui Qt6::Widgets)
