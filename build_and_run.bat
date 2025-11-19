@echo off
echo Building SeamCarving project...

:: Create build directory
if not exist "build" mkdir build
cd build

:: Configure and build
echo Running CMake...
cmake ..
echo Building project...
cmake --build . --config Release

:: Copy DLLs if needed
echo Copying OpenCV DLLs...
if exist "..\opencv\build\x64\vc16\bin\opencv_world*.dll" (
    copy "..\opencv\build\x64\vc16\bin\opencv_world*.dll" ".\Release\" >nul
)

:: Copy test image to Release folder
echo Copying test image...
if exist "..\images\test_image.jpg" (
    copy "..\images\test_image.jpg" ".\Release\" >nul
    echo Test image copied successfully!
) else (
    echo Warning: test_image.jpg not found in images folder!
    echo Please make sure images/test_image.jpg exists
)

:: Run the program
echo Running program...
cd Release
SeamCarving.exe