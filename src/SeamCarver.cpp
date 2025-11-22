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

// ============================================================================
// DYNAMIC PROGRAMMING IMPLEMENTATION
// ============================================================================

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

    // Fill DP table row by row (top to bottom)
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

vector<int> SeamCarver::findHorizontalSeamDP() {
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

    // Backtrack table: stores which row in previous column led to minimum
    Mat backtrack(rows, cols, CV_32S);

    // Initialize first column with energy values
    for (int i = 0; i < rows; i++) {
        dp.at<double>(i, 0) = energy.at<float>(i, 0);
    }

    // Fill DP table column by column (left to right)
    for (int j = 1; j < cols; j++) {
        for (int i = 0; i < rows; i++) {
            // Start with pixel directly to the left
            double min_energy = dp.at<double>(i, j - 1);
            int min_row = i;

            // Check upper-left diagonal (if exists)
            if (i > 0) {
                double upper_left = dp.at<double>(i - 1, j - 1);
                if (upper_left < min_energy) {
                    min_energy = upper_left;
                    min_row = i - 1;
                }
            }

            // Check lower-left diagonal (if exists)
            if (i < rows - 1) {
                double lower_left = dp.at<double>(i + 1, j - 1);
                if (lower_left < min_energy) {
                    min_energy = lower_left;
                    min_row = i + 1;
                }
            }

            // Store cumulative energy and backtrack info
            dp.at<double>(i, j) = energy.at<float>(i, j) + min_energy;
            backtrack.at<int>(i, j) = min_row;
        }
    }

    // Find minimum energy in last column
    int min_row = 0;
    double min_energy = dp.at<double>(0, cols - 1);

    for (int i = 1; i < rows; i++) {
        if (dp.at<double>(i, cols - 1) < min_energy) {
            min_energy = dp.at<double>(i, cols - 1);
            min_row = i;
        }
    }

    // Backtrack to find the seam path
    vector<int> seam(cols);
    seam[cols - 1] = min_row;

    for (int j = cols - 2; j >= 0; j--) {
        seam[j] = backtrack.at<int>(seam[j + 1], j + 1);
    }

    return seam;
}

// ============================================================================
// GREEDY ALGORITHM IMPLEMENTATION
// TODO: Update dis shizz
// Current implementation: Simple greedy that picks minimum energy neighbor
// ============================================================================

vector<int> SeamCarver::findVerticalSeamGreedy() {
    Mat energy = computeEnergyMap();
    int rows = energy.rows;
    int cols = energy.cols;

    vector<int> seam(rows);

    //pick minimum energy pixel in first row
    float min_val = energy.at<float>(0, 0);
    int min_col = 0;
    for (int j = 1; j < cols; j++) {
        float val = energy.at<float>(0, j);
        if (val < min_val) {
            min_val = val;
            min_col = j;
        }
    }
    seam[0] = min_col;

    //row by row pick minimum of neighbors
    for (int i = 1; i < rows; i++) {
        int prev_col = seam[i - 1];
        int best_col = prev_col;
        float best_energy = energy.at<float>(i, prev_col);

        //top left
        if (prev_col > 0) {
            float leftE = energy.at<float>(i, prev_col - 1);
            if (leftE < best_energy) {
                best_energy = leftE;
                best_col = prev_col - 1;
            }
        }

        //top right
        if (prev_col < cols - 1) {
            float rightE = energy.at<float>(i, prev_col + 1);
            if (rightE < best_energy) {
                best_energy = rightE;
                best_col = prev_col + 1;
            }
        }

        seam[i] = best_col;
    }

    return seam;
}

vector<int> SeamCarver::findHorizontalSeamGreedy() {
    Mat energy = computeEnergyMap();
    int rows = energy.rows;
    int cols = energy.cols;

    vector<int> seam(cols);

    //pick minimum energy pixel in first column
    float min_val = energy.at<float>(0, 0);
    int min_row = 0;
    for (int i = 1; i < rows; i++) {
        float val = energy.at<float>(i, 0);
        if (val < min_val) {
            min_val = val;
            min_row = i;
        }
    }
    seam[0] = min_row;

    //column by column pick minimum neighbor
    for (int j = 1; j < cols; j++) {
        int prev_row = seam[j - 1];
        int best_row = prev_row;
        float best_energy = energy.at<float>(prev_row, j);

        //top left
        if (prev_row > 0) {
            float upE = energy.at<float>(prev_row - 1, j);
            if (upE < best_energy) {
                best_energy = upE;
                best_row = prev_row - 1;
            }
        }

        //bottom left
        if (prev_row < rows - 1) {
            float downE = energy.at<float>(prev_row + 1, j);
            if (downE < best_energy) {
                best_energy = downE;
                best_row = prev_row + 1;
            }
        }

        seam[j] = best_row;
    }

    return seam;
}

// ============================================================================
// SEAM REMOVAL FUNCTIONS
// ============================================================================

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

void SeamCarver::removeHorizontalSeam(const vector<int>& seam) {
    if (seam.size() != image_.cols) {
        cerr << "Error: Seam size (" << seam.size()
            << ") doesn't match image width (" << image_.cols << ")!" << endl;
        return;
    }

    if (image_.rows <= 1) {
        cerr << "Error: Image is too short to remove more seams!" << endl;
        return;
    }

    // Create new image with one less row
    Mat new_image(image_.rows - 1, image_.cols, image_.type());

    for (int j = 0; j < image_.cols; j++) {
        int seam_row = seam[j];

        // Validate seam position
        if (seam_row < 0 || seam_row >= image_.rows) {
            cerr << "Error: Invalid seam position at col " << j
                << ": " << seam_row << " (rows: " << image_.rows << ")" << endl;
            return;
        }

        // Copy all pixels except the seam pixel
        for (int i = 0; i < seam_row; i++) {
            new_image.at<Vec3b>(i, j) = image_.at<Vec3b>(i, j);
        }

        for (int i = seam_row + 1; i < image_.rows; i++) {
            new_image.at<Vec3b>(i - 1, j) = image_.at<Vec3b>(i, j);
        }
    }

    image_ = new_image;
}

// ============================================================================
// VISUALIZATION FUNCTIONS
// ============================================================================

Mat SeamCarver::visualizeVerticalSeam(const vector<int>& seam, const Scalar& color) {
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

Mat SeamCarver::visualizeHorizontalSeam(const vector<int>& seam, const Scalar& color) {
    if (seam.size() != image_.cols) {
        cerr << "Error: Seam size doesn't match image width in visualization!" << endl;
        return image_.clone();
    }

    Mat result = image_.clone();

    // Draw the seam with specified color
    for (int j = 0; j < seam.size(); j++) {
        int row = seam[j];
        if (row >= 0 && row < result.rows) {
            // Make seam more visible by drawing a thicker line
            result.at<Vec3b>(row, j) = Vec3b(color[0], color[1], color[2]);

            // Optional: make it thicker for better visibility
            if (row > 0) {
                Vec3b& pixel = result.at<Vec3b>(row - 1, j);
                pixel[0] = (pixel[0] + color[0]) / 2;
                pixel[1] = (pixel[1] + color[1]) / 2;
                pixel[2] = (pixel[2] + color[2]) / 2;
            }
            if (row < result.rows - 1) {
                Vec3b& pixel = result.at<Vec3b>(row + 1, j);
                pixel[0] = (pixel[0] + color[0]) / 2;
                pixel[1] = (pixel[1] + color[1]) / 2;
                pixel[2] = (pixel[2] + color[2]) / 2;
            }
        }
    }

    return result;
}
