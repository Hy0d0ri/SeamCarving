#include "SeamCarver.hpp"
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <limits>

using namespace cv;
using namespace std;

SeamCarver::SeamCarver(const Mat& image) : image_(image.clone()) {
    computeEnergyMap();
}

void SeamCarver::computeEnergyMap() {
    if (image_.empty()) {
        cerr << "Error: Image is empty in computeEnergyMap!" << endl;
        return;
    }

    Mat gray;

    // Convert to grayscale
    if (image_.channels() == 3) {
        cvtColor(image_, gray, COLOR_BGR2GRAY);
    }
    else {
        gray = image_.clone();
    }

    // Calculate gradients
    Mat grad_x, grad_y;
    Sobel(gray, grad_x, CV_64F, 1, 0, 3);
    Sobel(gray, grad_y, CV_64F, 0, 1, 3);

    // Convert to absolute values and combine
    Mat abs_grad_x, abs_grad_y;
    convertScaleAbs(grad_x, abs_grad_x);
    convertScaleAbs(grad_y, abs_grad_y);

    // Combine gradients
    addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, energy_);
}

vector<int> SeamCarver::findVerticalSeamDP() {
    if (energy_.empty()) {
        cerr << "Error: Energy map is empty!" << endl;
        return vector<int>();
    }

    int rows = energy_.rows;
    int cols = energy_.cols;

    if (rows == 0 || cols == 0) {
        cerr << "Error: Energy map has zero dimensions!" << endl;
        return vector<int>();
    }

    // Use double precision for DP to avoid overflow
    Mat dp(rows, cols, CV_64F);
    Mat backtrack(rows, cols, CV_32S);

    // Initialize first row
    for (int j = 0; j < cols; j++) {
        dp.at<double>(0, j) = static_cast<double>(energy_.at<uchar>(0, j));
        backtrack.at<int>(0, j) = j;
    }

    // Fill DP table
    for (int i = 1; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double min_energy = dp.at<double>(i - 1, j);
            int min_index = j;

            // Check left neighbor
            if (j > 0 && dp.at<double>(i - 1, j - 1) < min_energy) {
                min_energy = dp.at<double>(i - 1, j - 1);
                min_index = j - 1;
            }

            // Check right neighbor
            if (j < cols - 1 && dp.at<double>(i - 1, j + 1) < min_energy) {
                min_energy = dp.at<double>(i - 1, j + 1);
                min_index = j + 1;
            }

            dp.at<double>(i, j) = static_cast<double>(energy_.at<uchar>(i, j)) + min_energy;
            backtrack.at<int>(i, j) = min_index;
        }
    }

    // Find minimum in last row
    int min_j = 0;
    double min_energy = dp.at<double>(rows - 1, 0);
    for (int j = 1; j < cols; j++) {
        if (dp.at<double>(rows - 1, j) < min_energy) {
            min_energy = dp.at<double>(rows - 1, j);
            min_j = j;
        }
    }

    // Backtrack to find seam
    vector<int> seam(rows);
    seam[rows - 1] = min_j;
    for (int i = rows - 2; i >= 0; i--) {
        seam[i] = backtrack.at<int>(i + 1, seam[i + 1]);
    }

    return seam;
}

vector<int> SeamCarver::findVerticalSeamGreedy() {
    if (energy_.empty()) {
        cerr << "Error: Energy map is empty!" << endl;
        return vector<int>();
    }

    int rows = energy_.rows;
    int cols = energy_.cols;

    if (rows == 0 || cols == 0) {
        cerr << "Error: Energy map has zero dimensions!" << endl;
        return vector<int>();
    }

    vector<int> seam(rows);

    // Start with minimum energy pixel in first row
    int min_j = 0;
    for (int j = 1; j < cols; j++) {
        if (energy_.at<uchar>(0, j) < energy_.at<uchar>(0, min_j)) {
            min_j = j;
        }
    }
    seam[0] = min_j;

    // Greedy selection for subsequent rows
    for (int i = 1; i < rows; i++) {
        int prev_j = seam[i - 1];
        int best_j = prev_j;
        uchar best_energy = energy_.at<uchar>(i, prev_j);

        // Check left neighbor
        if (prev_j > 0) {
            uchar left_energy = energy_.at<uchar>(i, prev_j - 1);
            if (left_energy < best_energy) {
                best_energy = left_energy;
                best_j = prev_j - 1;
            }
        }

        // Check right neighbor
        if (prev_j < cols - 1) {
            uchar right_energy = energy_.at<uchar>(i, prev_j + 1);
            if (right_energy < best_energy) {
                best_j = prev_j + 1;
            }
        }

        seam[i] = best_j;
    }

    return seam;
}

void SeamCarver::removeVerticalSeam(const vector<int>& seam) {
    if (seam.size() != image_.rows) {
        cerr << "Error: Seam size doesn't match image height!" << endl;
        return;
    }

    if (image_.cols <= 1) {
        cerr << "Error: Image is too narrow to remove more seams!" << endl;
        return;
    }

    Mat new_image(image_.rows, image_.cols - 1, image_.type());

    for (int i = 0; i < image_.rows; i++) {
        int seam_pos = seam[i];

        // Validate seam position
        if (seam_pos < 0 || seam_pos >= image_.cols) {
            cerr << "Error: Invalid seam position at row " << i << ": " << seam_pos << endl;
            return;
        }

        // Copy pixels before seam
        if (seam_pos > 0) {
            image_.row(i).colRange(0, seam_pos).copyTo(
                new_image.row(i).colRange(0, seam_pos));
        }

        // Copy pixels after seam
        if (seam_pos < image_.cols - 1) {
            image_.row(i).colRange(seam_pos + 1, image_.cols).copyTo(
                new_image.row(i).colRange(seam_pos, new_image.cols));
        }
    }

    image_ = new_image;
    computeEnergyMap(); // Recompute energy after removing seam
}

Mat SeamCarver::visualizeSeam(const vector<int>& seam, const Scalar& color) {
    if (seam.size() != image_.rows) {
        cerr << "Error: Seam size doesn't match image height in visualization!" << endl;
        return image_.clone();
    }

    Mat result = image_.clone();
    for (int i = 0; i < seam.size(); i++) {
        int col = seam[i];
        if (col >= 0 && col < result.cols) {
            result.at<Vec3b>(i, col) = Vec3b(color[0], color[1], color[2]);
        }
    }
    return result;
}