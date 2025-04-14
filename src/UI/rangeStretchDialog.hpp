#pragma once

#include <optional>
#include <qtypes.h>
#include <qwidget.h>
#include <tuple>

std::optional<std::tuple<uchar, uchar, uchar, uchar>>
getParametersDialog(QWidget *parent, uchar initialP1, uchar initialP2,
                    uchar initialQ3, uchar initialQ4);
