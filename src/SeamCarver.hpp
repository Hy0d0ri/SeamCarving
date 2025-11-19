#ifndef SEAM_CARVER_HPP
#define SEAM_CARVER_HPP

#include <opencv2/opencv.hpp>
#include <vector>

class SeamCarver {
public:
    SeamCarver(const cv::Mat& image);

    // Core seam carving functions
    std::vector<int> findVerticalSeamDP();
    std::vector<int> findVerticalSeamGreedy(); // TODO: Implement greedy algorithm

    void removeVerticalSeam(const std::vector<int>& seam);
    cv::Mat visualizeSeam(const std::vector<int>& seam, const cv::Scalar& color = cv::Scalar(0, 0, 255));

    // Getters
    cv::Mat getImage() const { return image_; }
    cv::Mat getEnergyMap() const { return energy_; }
    int getWidth() const { return image_.cols; }
    int getHeight() const { return image_.rows; }

private:
    cv::Mat image_;
    cv::Mat energy_;

    void computeEnergyMap();
};

#endif