#pragma once

#include <optional>
#include <qtypes.h>
#include <qwidget.h>
#include <tuple>

std::optional<std::tuple<uchar, uchar, uchar, uchar>>
rangeStretchDialog(QWidget *parent, uchar initialP1, uchar initialP2,
                    uchar initialQ3, uchar initialQ4);

std::optional<uchar> posterizeDialog(QWidget *parent, uchar N);
std::optional<uchar> kernelSizeDialog(QWidget *parent, uchar i, std::vector<uchar> allowed_sizes);

std::optional<std::tuple<uchar, double>>
gaussianBlurDialog(QWidget *parent, uchar i, std::vector<uchar> allowed_sizes);
