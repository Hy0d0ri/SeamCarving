About this project:
TBA

Setup Guide:
1. Once pulled from GitHub, ensure that the images folder in the project root folder has an image named test_image.jpg. This will be the image that will be sent to the program after building and running.

2. In CMakeLists.txt in the project root folder, look for a line that resembles this:

set(OpenCV_DIR "D:/School/Year 3 Tri 1/Algorithm Analysis/Ass2Final/SeamCarving/opencv/build" CACHE PATH "OpenCV directory")

Change the file path (everything within the "" to your local project's SeamCarving/opencv/build and take note of which '\' or '/' you are using, it must be like the one above.

3. Click on the build_and_run.bat file in the root folder, this should open up the program properly.

Editing Guide:
1. Only make changes to the scripts in the SeamCarving/src folder.

2. Once changes are made, run the build_and_run.bat file again to rebuild and your changes should be observable immediately.

3. When pushing to main do NOT push changes to CMakeLists.txt or the build folder if it happens to show up.