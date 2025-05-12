#include "utils.hpp"
#include <opencv2/opencv.hpp>
#include <stdexcept>

cv::Mat StructuringElement::select(uint index) {
	if (index > 1) throw new std::invalid_argument("Invalid index");
	auto [shape, size] = values[index];
	return cv::getStructuringElement(shape, size);
}
