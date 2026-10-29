#include "common.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
int main() {
  QT_BEGIN("opencv");
  cv::Mat img = cv::Mat::zeros(64, 64, CV_8UC1);
  cv::rectangle(img, cv::Rect(16, 16, 32, 32), cv::Scalar(255), cv::FILLED);
  QT_CHECK(img.rows == 64 && img.cols == 64, "Mat geometry");
  QT_CHECK(img.at<uchar>(32, 32) == 255, "rectangle drawn");
  QT_CHECK(img.at<uchar>(2, 2) == 0, "background untouched");
  cv::Mat blurred;
  cv::GaussianBlur(img, blurred, cv::Size(5, 5), 1.5);
  QT_CHECK(blurred.size() == img.size(), "GaussianBlur preserves size");
  cv::Mat edges;
  cv::Canny(img, edges, 50, 150);
  QT_CHECK(cv::countNonZero(edges) > 0, "Canny found edges");
  cv::Mat a = (cv::Mat_<double>(2, 2) << 1, 2, 3, 4);
  QT_CHECK(std::abs(cv::determinant(a) - (-2.0)) < 1e-12, "determinant");
  printf("  info opencv %s\n", CV_VERSION);
  QT_END();
}
