#pragma once

#include "dialogs/utils.hpp"
#include <opencv2/core/base.hpp>
#include <optional>
#include <qwidget.h>
#include <tuple>

std::optional<std::tuple<uchar, uchar, uchar, uchar>> rangeStretchDialog(QWidget *parent);

std::optional<uchar> posterizeDialog(QWidget *parent);

std::optional<std::tuple<KernelSizes::ValueType, BorderTypes::ValueType>>
kernelBorderDialog(QWidget *parent);

std::optional<std::tuple<KernelSizes::ValueType, BorderTypes::ValueType, double>>
gaussianBlurDialog(QWidget *parent);

std::optional<std::tuple<KernelSizes::ValueType, BorderTypes::ValueType, SobelDirections::Enum>>
sobelDialog(QWidget *parent);

std::optional<std::tuple<KernelSizes::ValueType, BorderTypes::ValueType, uchar, uchar>>
cannyDialog(QWidget *parent);

std::optional<std::tuple<cv::Mat, BorderTypes::ValueType>>
choosableMaskDialog(QWidget *parent, const std::vector<cv::Mat> &mats, const std::vector<QString> &names);

std::optional<std::tuple<cv::Mat, BorderTypes::ValueType>>
twoStageFilterDialog(QWidget *parent);

std::optional<std::tuple<uint, uint>>
windowsPairDialog(QWidget *parent, const std::vector<QString> &names, uint activeWindowIndex);

std::optional<std::tuple<uint, uint, uchar>>
windowsPairBlendDialog(QWidget *parent, const std::vector<QString> &names, uint activeWindowIndex);
