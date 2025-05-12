#pragma once
#include "DialogBuilder.hpp"
#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

#include <QComboBox>
#include <QFormLayout>
#include <QIntValidator>
#include <QLineEdit>
#include <QMessageBox>
#include <QString>
#include <opencv2/core/base.hpp>
#include <opencv2/core/mat.hpp>
#include <optional>
#include <qcombobox.h>
#include <qgridlayout.h>
#include <qlabel.h>
#include <tuple>

namespace BorderTypes {
using ValueType = int;
const std::vector<ValueType> values = {
    cv::BORDER_CONSTANT,    //
    cv::BORDER_REPLICATE,   //
    cv::BORDER_REFLECT,     //
    cv::BORDER_REFLECT_101, //
    cv::BORDER_ISOLATED,    //
};
const std::vector<QString> strings{
    "Constant (000000|abcdefgh|0000000)",
    "Replicate (aaaaaa|abcdefgh|hhhhhhh)",
    "Reflect (fedcba|abcdefgh|hgfedcb)",
    "Reflect 101 (gfedcb|abcdefgh|gfedcba)",
    "Isolated",
};
const auto inputSpec = InputSpec<DialogValue::EnumVariant>{"Border type", {strings}, 3};
constexpr ValueType select(uint index) { return values[index]; }
} // namespace BorderTypes

namespace KernelSizes {
using ValueType = uchar;
const std::vector<ValueType> values{1, 3, 5, 7};
const std::vector<QString> strings{"1x1", "3x3", "5x5", "7x7"};
const auto inputSpec = InputSpec<DialogValue::EnumVariant>{"Kernel size", {strings}, 1};
constexpr ValueType select(uint index) { return values[index]; }
} // namespace KernelSizes

namespace SobelDirections {
enum Enum {
  Horizontal,
  Vertical,
};
const std::vector<Enum> values{Enum::Horizontal, Enum::Vertical};
const std::vector<QString> strings{"Horizontal", "Vertical"};
const auto inputSpec = InputSpec<DialogValue::EnumVariant>{"Direction", {strings}, 0};
constexpr Enum select(uint index) { return values[index]; }
} // namespace SobelDirections

namespace BoxKernel {
const cv::Mat mat3 = (cv::Mat_<double>(3, 3) << 1, 1, 1, 1, 1, 1, 1, 1, 1);
}

namespace LaplacianMasks {
const std::vector<cv::Mat> mats = {(cv::Mat_<double>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0),
                                   (cv::Mat_<double>(3, 3) << -1, -1, -1, -1, 9, -1, -1, -1, -1),
                                   (cv::Mat_<double>(3, 3) << 1, -2, 1, -2, 5, -2, 1, -2, 1)};
const std::vector<QString> names{"1", "2", "3"};
} // namespace LaplacianMasks

namespace PrewittMasks {
const std::vector<cv::Mat> mats = {
    // N
    (cv::Mat_<double>(3, 3) << -1, -1, -1, 0, 0, 0, 1, 1, 1),
    // NE
    (cv::Mat_<double>(3, 3) << 0, -1, -1, 1, 0, -1, 1, 1, 0),
    // E
    (cv::Mat_<double>(3, 3) << 1, 0, -1, 1, 0, -1, 1, 0, -1),
    // SE
    (cv::Mat_<double>(3, 3) << 1, 1, 0, 1, 0, -1, 0, -1, -1),
    // S
    (cv::Mat_<double>(3, 3) << 1, 1, 1, 0, 0, 0, -1, -1, -1),
    // SW
    (cv::Mat_<double>(3, 3) << 0, 1, 1, -1, 0, 1, -1, -1, 0),
    // W
    (cv::Mat_<double>(3, 3) << -1, 0, 1, -1, 0, 1, -1, 0, 1),
    // NW
    (cv::Mat_<double>(3, 3) << -1, -1, 0, -1, 0, 1, 0, 1, 1),
};
const std::vector<QString> names{"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
} // namespace PrewittMasks

namespace StructuringElement {
const std::vector<std::tuple<cv::MorphShapes, cv::Size>> values = {
    {cv::MorphShapes::MORPH_CROSS, cv::Size(3, 3)}, {cv::MorphShapes::MORPH_RECT, cv::Size(3,3)}};
const std::vector<QString> names{"Cross 3x3", "Square 3x3"};
const auto inputSpec = InputSpec<DialogValue::EnumVariant>{"Structuring element", {names}, 0};
cv::Mat select(uint index);
} // namespace StructuringElement

namespace DialogResultsUtils {
template <typename Tuple, typename... Funcs, std::size_t... Is>
auto mapTupleImpl(const Tuple &tuple, std::index_sequence<Is...>, Funcs &&...funcs) {
  return std::make_tuple(std::forward<Funcs>(funcs)(std::get<Is>(tuple))...);
}

template <typename... Results, typename... Funcs>
auto mapTuple(const std::tuple<Results...> &tuple, Funcs &&...funcs) {
  static_assert(sizeof...(Results) == sizeof...(Funcs),
                "Number of functions must match number of tuple elements");
  return mapTupleImpl(tuple, std::index_sequence_for<Results...>{}, std::forward<Funcs>(funcs)...);
}

template <typename... Results, typename... Funcs>
auto mapTuple(const std::optional<std::tuple<Results...>> &tuple, Funcs &&...funcs)
    -> std::optional<std::tuple<std::invoke_result_t<Funcs, Results>...>> {
  if (!tuple.has_value())
    return std::nullopt;
  return mapTuple(tuple.value(), funcs...);
}

template <typename Result>
std::optional<Result> unpackSingularTuple(const std::optional<std::tuple<Result>> &result) {
  if (!result)
    return std::nullopt;

  auto [value] = result.value();
  return value;
}
} // namespace DialogResultsUtils
