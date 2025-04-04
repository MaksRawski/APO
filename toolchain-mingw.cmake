set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

set(Qt6_DIR /opt/Qt/6.8.2/mingw_64/lib/cmake/Qt6)
set(OpenCV_INCLUDE_DIRS /opt/windows-opencv/build/include)
set(OpenCV_DLLS_DIR /opt/windows-opencv-build/bin)

set(OpenCV_LIBS
    -L${OpenCV_DLLS_DIR}
    -lopencv_core4110
    -lopencv_highgui4110
    -lopencv_imgcodecs4110
    -lopencv_imgproc4110
    -lopencv_features2d4110
    -lopencv_calib3d4110
)

set(CMAKE_FIND_ROOT_PATH
    /usr/x86_64-w64-mingw32
    /opt/Qt
)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
