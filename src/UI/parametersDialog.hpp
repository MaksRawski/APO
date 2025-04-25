#pragma once

#include <opencv2/core/base.hpp>
#include <optional>
#include <qwidget.h>
#include <tuple>

std::optional<std::tuple<uchar, uchar, uchar, uchar>>
rangeStretchDialog(QWidget *parent, uchar initialP1, uchar initialP2, uchar initialQ3,
                   uchar initialQ4);

std::optional<uchar> posterizeDialog(QWidget *parent, uchar N);
std::optional<std::tuple<uchar, int>> kernelSizeDialog(QWidget *parent);

std::optional<std::tuple<uchar, double, int>> gaussianBlurDialog(QWidget *parent);
enum class Direction {
  Horizontal,
  Vertical,
};

std::optional<std::tuple<uchar, Direction, int>> sobelDialog(QWidget *parent);
std::optional<std::tuple<uchar, uchar, uchar, int>> cannyDialog(QWidget *parent);

using Mask3x3 = std::array<std::array<double, 3>, 3>;

std::optional<std::tuple<Mask3x3, int>> laplacianMaskDialog(QWidget *parent);
std::optional<std::tuple<Mask3x3, int>> prewittDirection(QWidget *parent);

std::optional<std::tuple<uchar, uchar>> windowsPairDialog(QWidget *parent,
                                                          std::vector<QString> names, int activeWindowIndex);
std::optional<std::tuple<uchar, uchar, uchar>>
windowsPairBlendDialog(QWidget *parent, std::vector<QString> names, int activeWindowIndex);
