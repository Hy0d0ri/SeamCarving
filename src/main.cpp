#include <opencv2/opencv.hpp>
#include <iostream>
#include "SeamCarver.hpp"

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
    cout << "Seam Carving - DP vs Greedy Algorithm Comparison" << endl;
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

    // Algorithm mode: true = DP, false = Greedy
    bool use_dp = true;

    cout << "\nControls:" << endl;
    cout << "  M        - Toggle between DP and GREEDY algorithm" << endl;
    cout << "  V/SPACE  - Remove one VERTICAL seam" << endl;
    cout << "  H        - Remove one HORIZONTAL seam" << endl;
    cout << "  1        - Show next VERTICAL seam (red)" << endl;
    cout << "  2        - Show next HORIZONTAL seam (green)" << endl;
    cout << "  E        - Show energy map" << endl;
    cout << "  R        - Reset to original" << endl;
    cout << "  S        - Save current carved image" << endl;
    cout << "  Q/ESC    - Quit" << endl;
    cout << "=================================================" << endl;
    cout << "Current algorithm: DP" << endl;

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
        double max_height = DISPLAY_HEIGHT - 120;
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

        // Show algorithm mode with color coding
        string algo_text = use_dp ? "CARVED (DP)" : "CARVED (GREEDY)";
        Scalar algo_color = use_dp ? Scalar(100, 255, 100) : Scalar(100, 150, 255);

        putText(display, algo_text, Point(carved_x, 30),
            FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
        putText(display, format("Size: %dx%d", carved.cols, carved.rows),
            Point(carved_x, 60), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(200, 200, 200), 1);
        putText(display, format("V-Seams: %d | H-Seams: %d", vertical_seams_removed, horizontal_seams_removed),
            Point(carved_x, 90), FONT_HERSHEY_SIMPLEX, 0.6, algo_color, 1);

        // Add algorithm indicator
        string mode_text = use_dp ? "MODE: Dynamic Programming" : "MODE: Greedy Algorithm";
        putText(display, mode_text, Point(carved_x, 120),
            FONT_HERSHEY_SIMPLEX, 0.6, algo_color, 2);

        // Add controls at bottom
        int bottom_y = DISPLAY_HEIGHT - 30;
        putText(display, "M: Toggle Algo | V/H: Remove seam | 1/2: Preview | E: Energy | R: Reset | S: Save | Q: Quit",
            Point(20, bottom_y), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(150, 150, 150), 1);

        imshow(WINDOW_NAME, display);

        int key = waitKey(30);

        if (key == 27 || key == 'q' || key == 'Q') {  // ESC or Q
            cout << "Exiting..." << endl;
            break;
        }
        else if (key == 'm' || key == 'M') {  // Toggle algorithm mode
            use_dp = !use_dp;
            string new_mode = use_dp ? "Dynamic Programming (DP)" : "Greedy Algorithm";
            cout << "\n*** Algorithm switched to: " << new_mode << " ***\n" << endl;
        }
        else if (key == ' ' || key == 'v' || key == 'V') {  // Remove vertical seam
            if (carved.cols > 1) {
                SeamCarver temp_carver(carved);
                vector<int> seam;

                // Choose algorithm based on mode
                if (use_dp) {
                    seam = temp_carver.findVerticalSeamDP();
                }
                else {
                    seam = temp_carver.findVerticalSeamGreedy();
                }

                if (!seam.empty()) {
                    temp_carver.removeVerticalSeam(seam);
                    carved = temp_carver.getImage();
                    vertical_seams_removed++;

                    string algo = use_dp ? "DP" : "Greedy";
                    cout << "Vertical seam removed using " << algo << "! New size: "
                        << carved.cols << "x" << carved.rows
                        << " (V:" << vertical_seams_removed
                        << ", H:" << horizontal_seams_removed << ")" << endl;
                }
            }
            else {
                cout << "Image too narrow to remove more vertical seams!" << endl;
            }
        }
        else if (key == 'h' || key == 'H') {  // Remove horizontal seam
            if (carved.rows > 1) {
                SeamCarver temp_carver(carved);
                vector<int> seam;

                // Choose algorithm based on mode
                if (use_dp) {
                    seam = temp_carver.findHorizontalSeamDP();
                }
                else {
                    seam = temp_carver.findHorizontalSeamGreedy();
                }

                if (!seam.empty()) {
                    temp_carver.removeHorizontalSeam(seam);
                    carved = temp_carver.getImage();
                    horizontal_seams_removed++;

                    string algo = use_dp ? "DP" : "Greedy";
                    cout << "Horizontal seam removed using " << algo << "! New size: "
                        << carved.cols << "x" << carved.rows
                        << " (V:" << vertical_seams_removed
                        << ", H:" << horizontal_seams_removed << ")" << endl;
                }
            }
            else {
                cout << "Image too short to remove more horizontal seams!" << endl;
            }
        }
        else if (key == '1') {  // Visualize next vertical seam
            SeamCarver temp_carver(carved);
            vector<int> seam;

            if (use_dp) {
                seam = temp_carver.findVerticalSeamDP();
            }
            else {
                seam = temp_carver.findVerticalSeamGreedy();
            }

            if (!seam.empty()) {
                Mat seam_vis = temp_carver.visualizeVerticalSeam(seam, Scalar(0, 0, 255));

                string algo = use_dp ? "DP" : "Greedy";
                string window_title = "Next VERTICAL Seam - " + algo + " (Red)";
                namedWindow(window_title, WINDOW_NORMAL);
                resizeWindow(window_title, 600, 500);
                imshow(window_title, seam_vis);

                cout << "Showing next VERTICAL seam using " << algo << " (red)" << endl;
            }
        }
        else if (key == '2') {  // Visualize next horizontal seam
            SeamCarver temp_carver(carved);
            vector<int> seam;

            if (use_dp) {
                seam = temp_carver.findHorizontalSeamDP();
            }
            else {
                seam = temp_carver.findHorizontalSeamGreedy();
            }

            if (!seam.empty()) {
                Mat seam_vis = temp_carver.visualizeHorizontalSeam(seam, Scalar(0, 255, 0));

                string algo = use_dp ? "DP" : "Greedy";
                string window_title = "Next HORIZONTAL Seam - " + algo + " (Green)";
                namedWindow(window_title, WINDOW_NORMAL);
                resizeWindow(window_title, 600, 500);
                imshow(window_title, seam_vis);

                cout << "Showing next HORIZONTAL seam using " << algo << " (green)" << endl;
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
            string algo = use_dp ? "DP" : "Greedy";
            string output_path = format("carved_%s_V%d_H%d.jpg", algo.c_str(),
                vertical_seams_removed, horizontal_seams_removed);
            imwrite(output_path, carved);
            cout << "Saved carved image to: " << output_path << endl;
        }
    }

    destroyAllWindows();
    cout << "\nProgram ended." << endl;
    cout << "Algorithm used: " << (use_dp ? "Dynamic Programming" : "Greedy") << endl;
    cout << "Vertical seams removed: " << vertical_seams_removed << endl;
    cout << "Horizontal seams removed: " << horizontal_seams_removed << endl;
    return 0;
}
