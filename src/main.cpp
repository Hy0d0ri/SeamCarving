#include <opencv2/opencv.hpp>
#include <iostream>
#include "SeamCarver.hpp"

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
    cout << "Seam Carving - Dynamic Programming vs Greedy Algorithm" << endl;
    cout << "======================================================" << endl;

    // Load test image from file
    string image_path = "test_image.jpg";  // Change this to your image path
    Mat image = imread(image_path);

    if (image.empty()) {
        cout << "Error: Could not load test image from: " << image_path << endl;
        cout << "Please make sure 'test_image.jpg' is in the same folder as the executable." << endl;
        return -1;
    }

    cout << "Successfully loaded test image: " << image.cols << " x " << image.rows << endl;

    SeamCarver carver(image);
    Mat original = image.clone();

    cout << "\nControls:" << endl;
    cout << "1 - Show DP Seam on original" << endl;
    cout << "2 - Show Greedy Seam on original" << endl;
    cout << "3 - Remove 10 seams with DP and show comparison" << endl;
    cout << "4 - Remove 10 seams with Greedy and show comparison" << endl;
    cout << "e - Show Energy Map" << endl;
    cout << "r - Reset to original" << endl;
    cout << "q - Quit" << endl;

    Mat display = image.clone();
    string window_name = "Seam Carving - Press 1-4 for actions, q to quit LOL";

    // Use WINDOW_NORMAL instead of WINDOW_AUTOSIZE so we can resize
    namedWindow(window_name, WINDOW_NORMAL);
    resizeWindow(window_name, 800, 600); // Set initial window size
    imshow(window_name, display);

    while (true) {
        int key = waitKey(0);

        if (key == 'q') break;

        switch (key) {
        case '1': { // Show DP seam on original
            SeamCarver temp_carver(original);
            vector<int> seam = temp_carver.findVerticalSeamDP();
            display = temp_carver.visualizeSeam(seam, Scalar(0, 0, 255));
            cout << "Showing DP seam (Red) on original image" << endl;
            break;
        }

        case '2': { // Show Greedy seam on original
            SeamCarver temp_carver(original);
            vector<int> seam = temp_carver.findVerticalSeamGreedy();
            display = temp_carver.visualizeSeam(seam, Scalar(0, 255, 0));
            cout << "Showing Greedy seam (Green) on original image" << endl;
            break;
        }

        case '3': { // Remove multiple DP seams and show side-by-side
            SeamCarver working_carver(original);

            // Remove 10 seams
            for (int i = 0; i < 10; i++) {
                vector<int> seam = working_carver.findVerticalSeamDP();
                working_carver.removeVerticalSeam(seam);
            }

            // Create side-by-side comparison
            Mat comparison;
            hconcat(original, working_carver.getImage(), comparison);

            // Add labels
            putText(comparison, "Original", Point(10, 30),
                FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
            putText(comparison, "After 10 DP Seams", Point(original.cols + 10, 30),
                FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);

            display = comparison;
            cout << "Removed 10 DP seams. New size: " << working_carver.getWidth() << "x" << working_carver.getHeight() << endl;

            // Resize window to fit the comparison image
            resizeWindow(window_name, min(1200, display.cols), min(800, display.rows));
            break;
        }

        case '4': { // Remove multiple Greedy seams and show side-by-side
            SeamCarver working_carver(original);

            // Remove 10 seams
            for (int i = 0; i < 10; i++) {
                vector<int> seam = working_carver.findVerticalSeamGreedy();
                working_carver.removeVerticalSeam(seam);
            }

            // Create side-by-side comparison
            Mat comparison;
            hconcat(original, working_carver.getImage(), comparison);

            // Add labels
            putText(comparison, "Original", Point(10, 30),
                FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
            putText(comparison, "After 10 Greedy Seams", Point(original.cols + 10, 30),
                FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);

            display = comparison;
            cout << "Removed 10 Greedy seams. New size: " << working_carver.getWidth() << "x" << working_carver.getHeight() << endl;

            // Resize window to fit the comparison image
            resizeWindow(window_name, min(1200, display.cols), min(800, display.rows));
            break;
        }

        case 'e': { // Show energy map
            SeamCarver temp_carver(original);
            Mat energy_display;
            applyColorMap(temp_carver.getEnergyMap(), energy_display, COLORMAP_JET);

            // Resize energy map to match original if needed
            if (energy_display.size() != original.size()) {
                resize(energy_display, energy_display, original.size());
            }

            // Show energy map explanation
            Mat energy_with_text = energy_display.clone();
            putText(energy_with_text, "ENERGY MAP: Red=High Energy (Keep), Blue=Low Energy (Remove)",
                Point(10, 30), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 2);

            imshow("Energy Map", energy_with_text);
            cout << "Energy map displayed in separate window" << endl;
            cout << "Red/Yellow areas = Important content, Blue areas = Can be removed" << endl;
            continue; // Don't update main window
        }

        case 'r': { // Reset
            display = original.clone();
            cout << "Reset to original image" << endl;
            // Reset window size
            resizeWindow(window_name, 800, 600);
            break;
        }
        }

        imshow(window_name, display);
    }

    cout << "Program ended successfully!" << endl;
    return 0;
}