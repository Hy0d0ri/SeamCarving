#include <opencv2/opencv.hpp>
#include <iostream>
#include "SeamCarver.hpp"

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
    cout << "Seam Carving - Dynamic Programming Implementation" << endl;
    cout << "=================================================" << endl;

    // Load test image
    string image_path = "test_image.jpg";
    Mat original = imread(image_path);

    if (original.empty()) {
        cout << "Error: Could not load image from: " << image_path << endl;
        cout << "Please make sure 'test_image.jpg' is in the same folder." << endl;
        return -1;
    }

    cout << "Loaded image: " << original.cols << " x " << original.rows << endl;

    // Create seam carver with the original image
    Mat carved = original.clone();

    cout << "\nControls:" << endl;
    cout << "  V/SPACE  - Remove one VERTICAL seam using DP" << endl;
    cout << "  H        - Remove one HORIZONTAL seam using DP" << endl;
    cout << "  1        - Show next VERTICAL seam (red) without removing" << endl;
    cout << "  2        - Show next HORIZONTAL seam (green) without removing" << endl;
    cout << "  E        - Show energy map" << endl;
    cout << "  R        - Reset to original" << endl;
    cout << "  S        - Save current carved image" << endl;
    cout << "  Q/ESC    - Quit" << endl;
    cout << "=================================================" << endl;

    // Fixed window size for comparison view
    const int DISPLAY_WIDTH = 1200;
    const int DISPLAY_HEIGHT = 600;
    const string WINDOW_NAME = "Seam Carving: Original (Left) vs Carved (Right)";

    namedWindow(WINDOW_NAME, WINDOW_NORMAL);
    resizeWindow(WINDOW_NAME, DISPLAY_WIDTH, DISPLAY_HEIGHT);

    int vertical_seams_removed = 0;
    int horizontal_seams_removed = 0;

    while (true) {
        // Create side-by-side comparison
        Mat display;

        // Resize images to fit in half the display width while maintaining aspect ratio
        int target_width = DISPLAY_WIDTH / 2 - 20;

        Mat original_resized, carved_resized;
        double scale_orig = (double)target_width / original.cols;
        double scale_carved = (double)target_width / carved.cols;

        // Use minimum scale to ensure both fit in height too
        double max_height = DISPLAY_HEIGHT - 100;
        scale_orig = min(scale_orig, max_height / original.rows);
        scale_carved = min(scale_carved, max_height / carved.rows);

        resize(original, original_resized, Size(), scale_orig, scale_orig);
        resize(carved, carved_resized, Size(), scale_carved, scale_carved);

        // Create black canvas
        display = Mat::zeros(DISPLAY_HEIGHT, DISPLAY_WIDTH, CV_8UC3);

        // Calculate positions to center images vertically
        int y_offset_orig = (DISPLAY_HEIGHT - original_resized.rows) / 2;
        int y_offset_carved = (DISPLAY_HEIGHT - carved_resized.rows) / 2;

        // Place original on left
        original_resized.copyTo(display(Rect(10, y_offset_orig,
            original_resized.cols, original_resized.rows)));

        // Place carved on right
        int carved_x = DISPLAY_WIDTH / 2 + 10;
        carved_resized.copyTo(display(Rect(carved_x, y_offset_carved,
            carved_resized.cols, carved_resized.rows)));

        // Add labels and info
        putText(display, "ORIGINAL", Point(10, 30),
            FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
        putText(display, format("Size: %dx%d", original.cols, original.rows),
            Point(10, 60), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(200, 200, 200), 1);

        putText(display, "CARVED (DP)", Point(carved_x, 30),
            FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
        putText(display, format("Size: %dx%d", carved.cols, carved.rows),
            Point(carved_x, 60), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(200, 200, 200), 1);
        putText(display, format("V-Seams: %d | H-Seams: %d", vertical_seams_removed, horizontal_seams_removed),
            Point(carved_x, 90), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(100, 255, 100), 1);

        // Add controls at bottom
        int bottom_y = DISPLAY_HEIGHT - 30;
        putText(display, "V/SPACE: Vertical | H: Horizontal | 1/2: Preview | E: Energy | R: Reset | S: Save | Q: Quit",
            Point(20, bottom_y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(150, 150, 150), 1);

        imshow(WINDOW_NAME, display);

        int key = waitKey(30);

        if (key == 27 || key == 'q' || key == 'Q') {  // ESC or Q
            cout << "Exiting..." << endl;
            break;
        }
        else if (key == ' ' || key == 'v' || key == 'V') {  // SPACE or V - Remove vertical seam
            if (carved.cols > 1) {
                SeamCarver temp_carver(carved);
                vector<int> seam = temp_carver.findVerticalSeamDP();

                if (!seam.empty()) {
                    temp_carver.removeVerticalSeam(seam);
                    carved = temp_carver.getImage();
                    vertical_seams_removed++;
                    cout << "Vertical seam removed! New size: " << carved.cols << "x" << carved.rows
                        << " (V:" << vertical_seams_removed << ", H:" << horizontal_seams_removed << ")" << endl;
                }
            }
            else {
                cout << "Image too narrow to remove more vertical seams!" << endl;
            }
        }
        else if (key == 'h' || key == 'H') {  // H - Remove horizontal seam
            if (carved.rows > 1) {
                SeamCarver temp_carver(carved);
                vector<int> seam = temp_carver.findHorizontalSeamDP();

                if (!seam.empty()) {
                    temp_carver.removeHorizontalSeam(seam);
                    carved = temp_carver.getImage();
                    horizontal_seams_removed++;
                    cout << "Horizontal seam removed! New size: " << carved.cols << "x" << carved.rows
                        << " (V:" << vertical_seams_removed << ", H:" << horizontal_seams_removed << ")" << endl;
                }
            }
            else {
                cout << "Image too short to remove more horizontal seams!" << endl;
            }
        }
        else if (key == '1') {  // Visualize next vertical seam
            SeamCarver temp_carver(carved);
            vector<int> seam = temp_carver.findVerticalSeamDP();

            if (!seam.empty()) {
                Mat seam_vis = temp_carver.visualizeVerticalSeam(seam, Scalar(0, 0, 255));

                // Show in a separate window
                namedWindow("Next VERTICAL Seam Preview (Red)", WINDOW_NORMAL);
                resizeWindow("Next VERTICAL Seam Preview (Red)", 600, 500);
                imshow("Next VERTICAL Seam Preview (Red)", seam_vis);

                cout << "Showing next VERTICAL seam to be removed (red)" << endl;
            }
        }
        else if (key == '2') {  // Visualize next horizontal seam
            SeamCarver temp_carver(carved);
            vector<int> seam = temp_carver.findHorizontalSeamDP();

            if (!seam.empty()) {
                Mat seam_vis = temp_carver.visualizeHorizontalSeam(seam, Scalar(0, 255, 0));

                // Show in a separate window
                namedWindow("Next HORIZONTAL Seam Preview (Green)", WINDOW_NORMAL);
                resizeWindow("Next HORIZONTAL Seam Preview (Green)", 600, 500);
                imshow("Next HORIZONTAL Seam Preview (Green)", seam_vis);

                cout << "Showing next HORIZONTAL seam to be removed (green)" << endl;
            }
        }
        else if (key == 'e' || key == 'E') {  // Show energy map
            SeamCarver temp_carver(carved);
            Mat energy = temp_carver.getEnergyMap();

            // Normalize and apply colormap
            Mat energy_normalized;
            normalize(energy, energy_normalized, 0, 255, NORM_MINMAX);
            energy_normalized.convertTo(energy_normalized, CV_8U);

            Mat energy_color;
            applyColorMap(energy_normalized, energy_color, COLORMAP_JET);

            // Show in separate window
            namedWindow("Energy Map (Red=High, Blue=Low)", WINDOW_NORMAL);
            resizeWindow("Energy Map (Red=High, Blue=Low)", 600, 500);
            imshow("Energy Map (Red=High, Blue=Low)", energy_color);

            cout << "Energy map displayed (Blue=Low energy, Red=High energy)" << endl;
        }
        else if (key == 'r' || key == 'R') {  // Reset
            carved = original.clone();
            vertical_seams_removed = 0;
            horizontal_seams_removed = 0;
            cout << "Reset to original image" << endl;
        }
        else if (key == 's' || key == 'S') {  // Save
            string output_path = format("carved_output_V%d_H%d.jpg", vertical_seams_removed, horizontal_seams_removed);
            imwrite(output_path, carved);
            cout << "Saved carved image to: " << output_path << endl;
        }
    }

    destroyAllWindows();
    cout << "\nProgram ended." << endl;
    cout << "Vertical seams removed: " << vertical_seams_removed << endl;
    cout << "Horizontal seams removed: " << horizontal_seams_removed << endl;
    return 0;
}
