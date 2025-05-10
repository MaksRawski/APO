#include "parametersDialog.hpp"
#include "dialogs/DialogBuilder.hpp"
#include "dialogs/utils.hpp"

#include <QComboBox>
#include <QFormLayout>
#include <QIntValidator>
#include <QLineEdit>
#include <QMessageBox>
#include <QString>
#include <opencv2/core/base.hpp>
#include <optional>
#include <qcombobox.h>
#include <qgridlayout.h>
#include <qlabel.h>
#include <tuple>

using namespace DialogResultsUtils;

template <typename T> T id(T t) { return t; }

std::optional<std::tuple<uchar, uchar, uchar, uchar>> rangeStretchDialog(QWidget *parent) {
  auto d = Dialog(parent, QString("Enter parameters"), //
                  InputSpec<DialogValue::Byte>{"p1", {0, 255}, 0},
                  InputSpec<DialogValue::Byte>{"p2", {0, 255}, 255},
                  InputSpec<DialogValue::Byte>{"q3", {0, 255}, 0},
                  InputSpec<DialogValue::Byte>{"q4", {0, 255}, 255});
  return d.run();
}

std::optional<uchar> posterizeDialog(QWidget *parent) {
  auto d = Dialog(parent, QString("Enter number of levels"),
                  InputSpec<DialogValue::Byte>{"N", {1, 255}, 2});

  return unpackSingularTuple(d.run());
}

std::optional<std::tuple<KernelSizes::ValueType, BorderTypes::ValueType>>
kernelBorderDialog(QWidget *parent) {
  auto d = Dialog(parent, QString("Select kernel size and border type"), KernelSizes::inputSpec,
                  BorderTypes::inputSpec);

  return mapTuple(d.run(), KernelSizes::select, BorderTypes::select);
}

std::optional<std::tuple<KernelSizes::ValueType, BorderTypes::ValueType, double>>
gaussianBlurDialog(QWidget *parent) {
  auto d = Dialog(parent, QString("Enter Gaussian blur parameters"), KernelSizes::inputSpec,
                  BorderTypes::inputSpec, InputSpec<DialogValue::Double>{"σ (std dev)", {}, 1.0});

  return mapTuple(d.run(), KernelSizes::select, BorderTypes::select, id<double>);
}

std::optional<std::tuple<KernelSizes::ValueType, BorderTypes::ValueType, SobelDirections::Enum>>
sobelDialog(QWidget *parent) {
  auto d = Dialog(parent, QString("Enter Sobel's filter parameters"), KernelSizes::inputSpec,
                  BorderTypes::inputSpec, SobelDirections::inputSpec);

  return mapTuple(d.run(), KernelSizes::select, BorderTypes::select, SobelDirections::select);
}

std::optional<std::tuple<KernelSizes::ValueType, BorderTypes::ValueType, uchar, uchar>>
cannyDialog(QWidget *parent) {
  auto d =
      Dialog(parent, QString("Enter Canny's filter parameters"), KernelSizes::inputSpec,
             BorderTypes::inputSpec, InputSpec<DialogValue::Byte>{"Start (0-255)", {0, 255}, 0},
             InputSpec<DialogValue::Byte>{"End (0-255)", {0, 255}, 255});

  return mapTuple(d.run(), KernelSizes::select, BorderTypes::select, id<uchar>, id<uchar>);
}

std::optional<std::tuple<cv::Mat, BorderTypes::ValueType>>
choosableMaskDialog(QWidget *parent, const std::vector<cv::Mat> &mats, const std::vector<QString> &names) {
  auto d = Dialog(parent, QString("Select a mask"),
                  InputSpec<DialogValue::ChoosableMask>{"Masks", {mats, names}, mats[0]},
                  BorderTypes::inputSpec);

  return mapTuple(d.run(), id<cv::Mat>, BorderTypes::select);
}

std::optional<std::tuple<cv::Mat, BorderTypes::ValueType>>
twoStageFilterDialog(QWidget *parent) {
  auto d = Dialog(parent, QString("Convolve a mask"),
                  InputSpec<DialogValue::ComposableMask>{"Mask", KernelSizes::values, BoxKernel::mat3},
                  BorderTypes::inputSpec);

  return mapTuple(d.run(), id<cv::Mat>, BorderTypes::select);
}


std::optional<std::tuple<uint, uint>>
windowsPairDialog(QWidget *parent, const std::vector<QString> &names, uint activeWindowIndex){
  uint secondWindowIndex = (activeWindowIndex + 1) % names.size();
  auto d =
      Dialog(parent, QString("Select two windows"),                                           //
             InputSpec<DialogValue::EnumVariant>{"First window", {names}, activeWindowIndex}, //
             InputSpec<DialogValue::EnumVariant>{"Second window", {names}, secondWindowIndex});

  return d.run();
}

std::optional<std::tuple<uint, uint, uchar>>
windowsPairBlendDialog(QWidget *parent, const std::vector<QString> &names, uint activeWindowIndex) {
  uint secondWindowIndex = (activeWindowIndex + 1) % names.size();
  auto d =
      Dialog(parent, QString("Select two windows"),                                           //
             InputSpec<DialogValue::EnumVariant>{"First window", {names}, activeWindowIndex}, //
             InputSpec<DialogValue::EnumVariant>{"Second window", {names}, secondWindowIndex},
             InputSpec<DialogValue::Byte>{"Blend percentage", {0, 100}, 50});

  return d.run();
}
