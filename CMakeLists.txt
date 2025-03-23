cmake_minimum_required(VERSION 3.16)
project(APO LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Widgets)

add_executable(APO
  src/main.cpp

  src/UI/mainwindow.hpp               src/UI/mainwindow.cpp
  src/UI/mdiChild.hpp                 src/UI/mdiChild.cpp
  src/UI/imageLabel.hpp               src/UI/imageLabel.cpp
  src/UI/histogramWidget.hpp          src/UI/histogramWidget.cpp

  # Processing
  src/imageProcessor.hpp           src/imageProcessor.cpp
)

target_link_libraries(APO PRIVATE Qt6::Gui Qt6::Widgets)
