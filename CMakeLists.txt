cmake_minimum_required(VERSION 3.16)
project(APO LANGUAGES CXX)

# disabling AUTOMOC cause it doesn't work lol
set(CMAKE_AUTOMOC OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt6_wrap_cpp(MOC_SOURCES
  src/UI/mainwindow.hpp
  src/UI/mdiChild.hpp
  src/UI/imageLabel.hpp
  src/UI/histogramWidget.hpp
)

add_executable(APO
  src/main.cpp
  src/UI/mainwindow.cpp
  src/UI/mdiChild.cpp
  src/UI/imageLabel.cpp
  src/UI/histogramWidget.cpp
  src/imageProcessor.cpp
  ${MOC_SOURCES}
)

target_link_libraries(APO PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
