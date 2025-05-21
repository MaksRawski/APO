SET Qt6_DIR="C:\Qt\6.9.0\msvc2022_64\lib\cmake\"
cmake -Bbuild-win
cmake --build .\build-win --config Release
copy C:\opencv\build\x64\vc16\bin\opencv_world4110.dll build-win\Release\
C:\Qt\6.9.0\msvc2022_64\bin\windeployqt6.exe .\build-win\Release\APO.exe
