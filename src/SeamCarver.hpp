#ifndef SEAM_CARVER_HPP
#define SEAM_CARVER_HPP

#include <opencv2/opencv.hpp>
#include <vector>

class SeamCarver {
public:
    SeamCarver(const cv::Mat& image);

    // Dynamic Programming seam finding
    std::vector<int> findVerticalSeamDP();
    std::vector<int> findHorizontalSeamDP();

    // Greedy Algorithm seam finding (TODO)
    std::vector<int> findVerticalSeamGreedy();
    std::vector<int> findHorizontalSeamGreedy();

    // Remove seams from the image
    void removeVerticalSeam(const std::vector<int>& seam);
    void removeHorizontalSeam(const std::vector<int>& seam);

    // Visualize seams on the image
    cv::Mat visualizeVerticalSeam(const std::vector<int>& seam, const cv::Scalar& color = cv::Scalar(0, 0, 255));
    cv::Mat visualizeHorizontalSeam(const std::vector<int>& seam, const cv::Scalar& color = cv::Scalar(0, 255, 0));

    // Getters
    cv::Mat getImage() const { return image_.clone(); }
    cv::Mat getEnergyMap() const;
    int getWidth() const { return image_.cols; }
    int getHeight() const { return image_.rows; }

private:
    cv::Mat image_;

    // Compute energy map using gradient magnitude
    cv::Mat computeEnergyMap() const;
};

#endif