#include "SeamCarver.hpp"
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <limits>
#include <cmath>

using namespace cv;
using namespace std;

SeamCarver::SeamCarver(const Mat& image) : image_(image.clone()) {
    if (image_.empty()) {
        cerr << "Error: Cannot create SeamCarver with empty image!" << endl;
    }
}

Mat SeamCarver::computeEnergyMap() const {
    if (image_.empty()) {
        cerr << "Error: Image is empty in computeEnergyMap!" << endl;
        return Mat();
    }

    Mat gray;

    // Convert to grayscale if needed
    if (image_.channels() == 3) {
        cvtColor(image_, gray, COLOR_BGR2GRAY);
    }
    else {
        gray = image_.clone();
    }

    // Convert to float for better precision
    gray.convertTo(gray, CV_32F);

    // Compute gradients using Sobel operator
    Mat grad_x, grad_y;
    Sobel(gray, grad_x, CV_32F, 1, 0, 3);  // Horizontal gradient
    Sobel(gray, grad_y, CV_32F, 0, 1, 3);  // Vertical gradient

    // Compute gradient magnitude: sqrt(grad_x^2 + grad_y^2)
    Mat energy;
    magnitude(grad_x, grad_y, energy);

    return energy;
}

Mat SeamCarver::getEnergyMap() const {
    return computeEnergyMap();
}

vector<int> SeamCarver::findVerticalSeamDP() {
    Mat energy = computeEnergyMap();

    if (energy.empty()) {
        cerr << "Error: Energy map is empty!" << endl;
        return vector<int>();
    }

    int rows = energy.rows;
    int cols = energy.cols;

    if (rows == 0 || cols == 0) {
        cerr << "Error: Energy map has zero dimensions!" << endl;
        return vector<int>();
    }

    // DP table: stores minimum cumulative energy to reach each pixel
    Mat dp(rows, cols, CV_64F);

    // Backtrack table: stores which column in previous row led to minimum
    Mat backtrack(rows, cols, CV_32S);

    // Initialize first row with energy values
    for (int j = 0; j < cols; j++) {
        dp.at<double>(0, j) = energy.at<float>(0, j);
    }

    // Fill DP table row by row
    for (int i = 1; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            // Start with pixel directly above
            double min_energy = dp.at<double>(i - 1, j);
            int min_col = j;

            // Check upper-left diagonal (if exists)
            if (j > 0) {
                double upper_left = dp.at<double>(i - 1, j - 1);
                if (upper_left < min_energy) {
                    min_energy = upper_left;
                    min_col = j - 1;
                }
            }

            // Check upper-right diagonal (if exists)
            if (j < cols - 1) {
                double upper_right = dp.at<double>(i - 1, j + 1);
                if (upper_right < min_energy) {
                    min_energy = upper_right;
                    min_col = j + 1;
                }
            }

            // Store cumulative energy and backtrack info
            dp.at<double>(i, j) = energy.at<float>(i, j) + min_energy;
            backtrack.at<int>(i, j) = min_col;
        }
    }

    // Find minimum energy in last row
    int min_col = 0;
    double min_energy = dp.at<double>(rows - 1, 0);

    for (int j = 1; j < cols; j++) {
        if (dp.at<double>(rows - 1, j) < min_energy) {
            min_energy = dp.at<double>(rows - 1, j);
            min_col = j;
        }
    }

    // Backtrack to find the seam path
    vector<int> seam(rows);
    seam[rows - 1] = min_col;

    for (int i = rows - 2; i >= 0; i--) {
        seam[i] = backtrack.at<int>(i + 1, seam[i + 1]);
    }

    return seam;
}

void SeamCarver::removeVerticalSeam(const vector<int>& seam) {
    if (seam.size() != image_.rows) {
        cerr << "Error: Seam size (" << seam.size()
            << ") doesn't match image height (" << image_.rows << ")!" << endl;
        return;
    }

    if (image_.cols <= 1) {
        cerr << "Error: Image is too narrow to remove more seams!" << endl;
        return;
    }

    // Create new image with one less column
    Mat new_image(image_.rows, image_.cols - 1, image_.type());

    for (int i = 0; i < image_.rows; i++) {
        int seam_col = seam[i];

        // Validate seam position
        if (seam_col < 0 || seam_col >= image_.cols) {
            cerr << "Error: Invalid seam position at row " << i
                << ": " << seam_col << " (cols: " << image_.cols << ")" << endl;
            return;
        }

        // Copy all pixels except the seam pixel
        for (int j = 0; j < seam_col; j++) {
            new_image.at<Vec3b>(i, j) = image_.at<Vec3b>(i, j);
        }

        for (int j = seam_col + 1; j < image_.cols; j++) {
            new_image.at<Vec3b>(i, j - 1) = image_.at<Vec3b>(i, j);
        }
    }

    image_ = new_image;
}

Mat SeamCarver::visualizeSeam(const vector<int>& seam, const Scalar& color) {
    if (seam.size() != image_.rows) {
        cerr << "Error: Seam size doesn't match image height in visualization!" << endl;
        return image_.clone();
    }

    Mat result = image_.clone();

    // Draw the seam with specified color
    for (int i = 0; i < seam.size(); i++) {
        int col = seam[i];
        if (col >= 0 && col < result.cols) {
            // Make seam more visible by drawing a thicker line
            result.at<Vec3b>(i, col) = Vec3b(color[0], color[1], color[2]);

            // Optional: make it thicker for better visibility
            if (col > 0) {
                Vec3b& pixel = result.at<Vec3b>(i, col - 1);
                pixel[0] = (pixel[0] + color[0]) / 2;
                pixel[1] = (pixel[1] + color[1]) / 2;
                pixel[2] = (pixel[2] + color[2]) / 2;
            }
            if (col < result.cols - 1) {
                Vec3b& pixel = result.at<Vec3b>(i, col + 1);
                pixel[0] = (pixel[0] + color[0]) / 2;
                pixel[1] = (pixel[1] + color[1]) / 2;
                pixel[2] = (pixel[2] + color[2]) / 2;
            }
        }
    }

    return result;
}