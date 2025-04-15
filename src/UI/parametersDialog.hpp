#pragma once

#include <opencv2/core/base.hpp>
#include <optional>
#include <qtypes.h>
#include <qwidget.h>
#include <tuple>

std::optional<std::tuple<uchar, uchar, uchar, uchar>>
rangeStretchDialog(QWidget *parent, uchar initialP1, uchar initialP2,
                   uchar initialQ3, uchar initialQ4);

std::optional<uchar> posterizeDialog(QWidget *parent, uchar N);
std::optional<std::tuple<uchar, int>> kernelSizeDialog(QWidget *parent);

std::optional<std::tuple<uchar, double, int>> gaussianBlurDialog(QWidget *parent);
enum class Direction {
  Horizontal,
  Vertical,
};

std::optional<std::tuple<uchar, Direction, int>> sobelDialog(QWidget *parent);
std::optional<std::tuple<uchar, uchar, uchar, int>> cannyDialog(QWidget *parent);
