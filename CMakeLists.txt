cmake_minimum_required(VERSION 3.16)
project(APO LANGUAGES CXX)

# AUTOMOC failed when toolchain-mingw.cmake was used
# so let's instead define MOC_SOURCES directly
set(CMAKE_AUTOMOC OFF)

set(Qt6_LIBS Qt6::Core Qt6::Gui Qt6::Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
if(WIN32)
  set(Qt6_DIR C:/Qt/6.9.0/msvc_2022_64/lib/cmake)
  set(OpenCV_DIR C:/opencv/build/x64/vc16/lib)
endif()
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

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
  src/UI/rangeStretchDialog.cpp
  src/imageProcessor.cpp
  src/imageWrapper.cpp
  ${MOC_SOURCES}
)

target_link_libraries(APO PRIVATE ${Qt6_LIBS} ${OpenCV_LIBS})

option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
  # Google Test configuration with FetchContent
  include(FetchContent)
  FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG v1.13.0
  )

  # Prevent GoogleTest from overriding our compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()
  function(add_gtest_executable TARGET_NAME)
    qt_add_executable(${TARGET_NAME}
      ${ARGN}
    )

    target_include_directories(${TARGET_NAME} PRIVATE
      ${CMAKE_SOURCE_DIR}/src
      ${Qt6Core_INCLUDE_DIRS}
      ${Qt6Gui_INCLUDE_DIRS}
      ${OpenCV_INCLUDE_DIRS}
      ${GTEST_INCLUDE_DIRS}
    )

    target_link_libraries(${TARGET_NAME} PRIVATE
      ${Qt6_LIBS}
      ${OpenCV_LIBS}
      gtest
      gtest_main
    )
  endfunction()

  add_gtest_executable(image_wrapper_tests
    tests/imageWrapperTest.cpp
    src/imageWrapper.cpp
  )
  add_test(
    NAME ImageWrapperTest
    COMMAND image_wrapper_tests
  )

  add_gtest_executable(image_processor_tests
    tests/imageProcessorTests.cpp
    src/imageWrapper.cpp
    src/imageProcessor.cpp
  )
  add_test(
    NAME ImageProcessorTest
    COMMAND image_processor_tests
  )
endif()
