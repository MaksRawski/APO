#pragma once

#include "../../imageWrapper.hpp"
#include "../ImageViewer.hpp"
#include "MaskEditor.hpp"
#include <QMessageBox>
#include <QPushButton>
#include <opencv2/core/base.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>
#include <optional>
#include <qboxlayout.h>
#include <qcombobox.h>
#include <qcontainerfwd.h>
#include <qdialog.h>
#include <qdialogbuttonbox.h>
#include <qformlayout.h>
#include <qlineedit.h>
#include <qobject.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtpreprocessorsupport.h>
#include <qtypes.h>
#include <qvalidator.h>
#include <qwidget.h>
#include <tuple>
#include <vector>

#define DELETE_DEFAULT_CONSTRUCTORS(Type) Type() = delete;

// Goal of each dialog is to get some values from the user.
// This enum holds all possible abstract types of values that a dialog
// may want to get from the user.
enum class DialogValue {
  Int,            // an integer with range restriction
  Double,         // a double precision value with no restrictions whatsoever
  EnumVariant,    // any variant chosen from a provided list
  ComposableMask, // a 5x5 mask that is achieved by convolution of two 3x3 masks
  ChoosableMask,  // a mask that can be chosen from a list and modified by the user
  ROI,            // a rectangle as a Region Of Interest
};

// This weird template struct allows for associating data
// with each variant of the DialogValue through specializations.
// Second template argument allows those specializations to also be generic.
template <DialogValue V, typename T> struct ValueTraits;

// ====
// Byte
// ====
struct IntRestriction {
  int low, high;
};
struct SteppedIntRestriction {
  int low, high, step;
};
template <typename R> struct ValueTraits<DialogValue::Int, R> {
  using Restriction = R;
  using RawResult = int;
  using MappedResult = RawResult;
};

// ======
// Double
// ======
template <> struct ValueTraits<DialogValue::Double, void> {
  using Restriction = struct {};
  using RawResult = double;
  using MappedResult = RawResult;
};

// =======
// Variant
// =======
struct EnumVariantRestriction {
  std::vector<QString> names;
  DELETE_DEFAULT_CONSTRUCTORS(EnumVariantRestriction);
};
template <typename T> struct ValueTraits<DialogValue::EnumVariant, T> {
  using Restriction = EnumVariantRestriction;
  using RawResult = uint;
  using MappedResult = T;
};

// ==============
// ComposableMask
// ==============
template <> struct ValueTraits<DialogValue::ComposableMask, void> {
  using Restriction = struct {};
  using RawResult = cv::Mat;
  using MappedResult = RawResult;
};

// =============
// ChoosableMask
// =============
struct ChoosableMasks {
  std::vector<cv::Mat> mats;
  std::vector<QString> descriptions;
  DELETE_DEFAULT_CONSTRUCTORS(ChoosableMasks);
};
template <> struct ValueTraits<DialogValue::ChoosableMask, void> {
  using Restriction = ChoosableMasks; // NOTE: this won't be a hard restriction!
  using RawResult = uint;
  using MappedResult = cv::Mat;
};

// ===
// ROI
// ===
template <> struct ValueTraits<DialogValue::ROI, void> {
  using Restriction = cv::Mat; // image from which ROI can be selected
  using RawResult = cv::Rect;
  using MappedResult = RawResult;
};

// InputSpec
template <DialogValue V, typename T> struct DialogParam {
  static constexpr DialogValue value = V;
  using MappedResult = typename ValueTraits<V, T>::MappedResult;
};

// generic InputSpec to be specialized later with `DialogParam`s
template <typename Param> struct InputSpec;

template <DialogValue V, typename T> struct InputSpec<DialogParam<V, T>> {
  using Restriction = typename ValueTraits<V, T>::Restriction;
  using RawResult = typename ValueTraits<V, T>::RawResult;
  using MappedResult = typename ValueTraits<V, T>::MappedResult;
  QString name;
  Restriction restriction;
  RawResult initialValue;
  DELETE_DEFAULT_CONSTRUCTORS(InputSpec);
};

// for EnumVariant a spec must additionally include a function which maps index to value
template <typename T> struct InputSpec<DialogParam<DialogValue::EnumVariant, T>> {
  using Restriction = typename ValueTraits<DialogValue::EnumVariant, T>::Restriction;
  using RawResult = typename ValueTraits<DialogValue::EnumVariant, T>::RawResult;
  using MappedResult = typename ValueTraits<DialogValue::EnumVariant, T>::MappedResult;
  QString name;
  Restriction restriction;
  RawResult initialValue;
  // function which maps chosen index to a value
  std::function<MappedResult(RawResult)> mapFn;
  DELETE_DEFAULT_CONSTRUCTORS(InputSpec);
};

// DialogParams
using IntParam = DialogParam<DialogValue::Int, IntRestriction>;
using SteppedIntParam = DialogParam<DialogValue::Int, SteppedIntRestriction>;
using DoubleParam = DialogParam<DialogValue::Double, void>;
template <typename T> using EnumVariantParam = DialogParam<DialogValue::EnumVariant, T>;
using ComposableMaskParam = DialogParam<DialogValue::ComposableMask, void>;
using ChoosableMaskParam = DialogParam<DialogValue::ChoosableMask, void>;
using ROIParam = DialogParam<DialogValue::ROI, void>;

QDialogButtonBox *createDialogButtons(QDialog *dialog);
QSpinBox *createValidatedIntEdit(QWidget *parent, int min, int max, int initialValue,
                                 int stepSize = 1);
QLineEdit *createValidatedDoubleEdit(QWidget *parent, int initialValue);
QComboBox *createComboBox(std::vector<QString> variants, uint initialIndex);

// Class for declarative creation of dialog windows.
template <typename... Params> class Dialog {
public:
  Dialog() = delete;

  template <typename Param> using RawResultType = typename Param::RawResult;
  template <typename Param> using ResultType = typename Param::MappedResult;

  using ResultTuple = std::tuple<ResultType<Params>...>;

  Dialog(QWidget *parent, QString title, InputSpec<Params>... inputs) {
    dialog = new QDialog(parent);
    dialog->setWindowTitle(title);

    QFormLayout *form = new QFormLayout(dialog);
    dialog->setLayout(form);
    // create appropriate inputs adding them to the form while returning "accessors"
    accessors = std::make_tuple(createInput(inputs, *form)...);

    previewImage = new ImageViewer(dialog);
    form->addRow(previewImage);

    QDialogButtonBox *buttons = createDialogButtons(dialog);
    form->addRow(buttons);

    auto invalidInput = [this, buttons]() {};
    paramChanged = [this, buttons]() {
      if (previewFn == nullptr)
        return;
      auto params = readParams();
      if (!params.has_value()) {
        previewImage->clear();
        buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
        return;
      }
      auto mat =
          std::apply([this](const auto &...param) { return previewFn(param...); }, params.value());
      if (!mat.has_value()) {
        previewImage->clear();
        buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
        return;
      }
      QPixmap pixmap = ImageWrapper(mat.value()).generateQPixmap();
      previewImage->setImage(pixmap);
      buttons->button(QDialogButtonBox::Ok)->setEnabled(true);
    };
  }

  std::optional<ResultTuple> readParams() const {
    auto optionals =
        std::apply([](auto &&...accessors) { return std::make_tuple(accessors()...); }, accessors);

    bool allOptionalsValid = true;
    std::apply([&](const auto &...opt) { ((allOptionalsValid &= opt.has_value()), ...); },
               optionals);

    if (!allOptionalsValid) {
      return std::nullopt;
    }

    ResultTuple results =
        std::apply([](const auto &...opt) { return std::make_tuple(opt.value()...); }, optionals);

    return results;
  }

  std::optional<ResultTuple> run() const {
    if (dialog->exec() != QDialog::Accepted)
      return std::nullopt;

    auto params = readParams();

    if (!params.has_value()) {
      QMessageBox::critical(dialog, "Error", "Invalid input.");
      return std::nullopt;
    }

    return params.value();
  }

  using PreviewFunction = std::function<std::optional<cv::Mat>(ResultType<Params>...)>;
  std::optional<cv::Mat> runWithPreview(PreviewFunction previewFn,
                                        PreviewFunction finalFn = nullptr) {
    if (finalFn == nullptr)
      finalFn = previewFn;

    this->previewFn = previewFn;
    dialog->resize(800, 800);
    paramChanged();

    auto params = run();
    if (!params.has_value())
      return std::nullopt;
    return std::apply([finalFn](const auto &...param) { return finalFn(param...); },
                      params.value());
  }

private:
  QDialog *dialog;
  ImageViewer *previewImage;
  std::function<void()> paramChanged = {};
  PreviewFunction previewFn = {};

  template <typename Param>
  using Accessor = std::function<std::optional<typename InputSpec<Param>::MappedResult>()>;

  std::tuple<Accessor<Params>...> accessors;

  Accessor<IntParam> //
  createInput(const InputSpec<IntParam> &spec, QFormLayout &form) {
    auto *edit = createValidatedIntEdit(dialog, spec.restriction.low, spec.restriction.high,
                                        spec.initialValue);
    form.addRow(spec.name, edit);
    QObject::connect(edit, &QSpinBox::textChanged, [this](auto _) { this->paramChanged(); });

    return [edit]() -> std::optional<int> { return static_cast<int>(edit->value()); };
  }

  Accessor<SteppedIntParam> //
  createInput(const InputSpec<SteppedIntParam> &spec, QFormLayout &form) {
    auto *edit = createValidatedIntEdit(dialog, spec.restriction.low, spec.restriction.high,
                                        spec.initialValue, spec.restriction.step);
    form.addRow(spec.name, edit);
    QObject::connect(edit, &QSpinBox::textChanged, [this](auto _) { this->paramChanged(); });

    return [edit]() -> std::optional<int> { return static_cast<int>(edit->value()); };
  }

  Accessor<DoubleParam> //
  createInput(const InputSpec<DoubleParam> &spec, QFormLayout &form) {
    auto *edit = createValidatedDoubleEdit(dialog, spec.initialValue);
    form.addRow(spec.name, edit);
    QObject::connect(edit, &QLineEdit::textChanged, [this](auto _) { this->paramChanged(); });

    return [edit]() -> std::optional<double> {
      bool ok;
      double v = static_cast<double>(edit->text().toDouble(&ok));
      if (ok)
        return v;
      else
        return std::nullopt;
    };
  }

  template <typename T>
  Accessor<EnumVariantParam<T>> //
  createInput(const InputSpec<EnumVariantParam<T>> &spec, QFormLayout &form) {
    auto *cb = createComboBox(spec.restriction.names, spec.initialValue);
    form.addRow(spec.name, cb);
    QObject::connect(cb, &QComboBox::currentIndexChanged, [this](auto _) { this->paramChanged(); });

    return [cb, spec]() -> std::optional<T> {
      int v = cb->currentIndex();
      if (v < 0)
        return std::nullopt;
      else {
        return spec.mapFn(v);
      }
    };
  }

  Accessor<ComposableMaskParam> //
  createInput(const InputSpec<ComposableMaskParam> &spec, QFormLayout &form) {
    cv::Mat initMat = spec.initialValue;
    auto mask1 = new MaskEditor(dialog, QSize(initMat.cols, initMat.rows));
    auto mask2 = new MaskEditor(dialog, QSize(initMat.cols, initMat.rows));
    mask1->setMask(initMat);
    mask2->setMask(initMat);

    auto maskRes = new MaskEditor(dialog, QSize(initMat.cols * 2 - 1, initMat.rows * 2 - 1));

    auto masksContainer = new QWidget(dialog);
    auto layout = new QHBoxLayout(masksContainer);
    layout->addWidget(mask1);
    layout->addWidget(mask2);
    masksContainer->setLayout(layout);
    form.addRow(masksContainer);

    form.addRow(maskRes);

    auto maskChanged = [mask1, mask2, maskRes]() {
      auto m1 = mask1->getMask();
      auto m2 = mask2->getMask();
      if (!m1.has_value() || !m2.has_value())
        return;

      cv::Mat k1 = m1.value();
      cv::Mat k2 = m2.value();
      k1.convertTo(k1, CV_32FC1);
      k2.convertTo(k2, CV_32FC1);

      cv::Mat k2_flipped;
      cv::flip(k2, k2_flipped, -1);

      int pad = k1.rows / 2;
      cv::Mat k1_padded;
      cv::copyMakeBorder(k1, k1_padded, pad, pad, pad, pad, cv::BORDER_CONSTANT, cv::Scalar(0));

      cv::Mat combined;
      cv::filter2D(k1_padded, combined, -1, k2_flipped, cv::Point(-1, -1), 0, cv::BORDER_CONSTANT);
      combined.convertTo(combined, CV_8SC1);

      maskRes->setMask(combined);
    };
    maskChanged();

    QObject::QObject::connect(mask1, &MaskEditor::maskChanged, maskChanged);
    QObject::QObject::connect(mask2, &MaskEditor::maskChanged, maskChanged);
    QObject::QObject::connect(maskRes, &MaskEditor::maskChanged,
                              [this]() { this->paramChanged(); });

    return [maskRes]() -> std::optional<cv::Mat> { return maskRes->getMask(); };
  }
  Accessor<ChoosableMaskParam> //
  createInput(const InputSpec<ChoosableMaskParam> &spec, QFormLayout &form) {
    bool cbVisible = true;
    QComboBox *cb;

    if (spec.restriction.descriptions.size() == 0) {
      cbVisible = false;
    } else {
      cb = createComboBox(spec.restriction.descriptions, 0);
      form.addRow(spec.name, cb);
    }

    cv::Mat mat = spec.restriction.mats[spec.initialValue];
    auto mask = new MaskEditor(dialog, QSize(mat.cols, mat.rows));
    mask->setMask(mat);
    form.addRow(mask);
    QObject::QObject::connect(mask, &MaskEditor::maskChanged, [this]() { this->paramChanged(); });

    if (cbVisible) {
      // we update the MaskEditor anytime a ComboBox is changed
      QObject::QObject::connect(cb, &QComboBox::currentIndexChanged, [mask, spec](uint index) {
        mask->setMask(spec.restriction.mats[index]);
      });
    }

    return [mask]() -> std::optional<cv::Mat> { return mask->getMask(); };
  }

  cv::Rect roi;

  Accessor<ROIParam> //
  createInput(const InputSpec<ROIParam> &spec, QFormLayout &form) {
    QPushButton *selectROI = new QPushButton(dialog);
    selectROI->setText("Select ROI");
    form.addRow("", selectROI);

    ImageViewer *imageViewer = new ImageViewer(dialog);
    imageViewer->setImage(ImageWrapper(spec.restriction).generateQPixmap());
    QObject::connect(imageViewer, &ImageViewer::roiSelected, [this, imageViewer](cv::Rect roi) {
      this->roi = roi;
      imageViewer->cancelGetROIFromUser();
      this->paramChanged();
    });

    QObject::connect(selectROI, &QPushButton::clicked, [imageViewer](bool _) {
      imageViewer->clearROI();
      imageViewer->getROIFromUser();
    });
    form.addRow("", imageViewer);

    return [this]() -> std::optional<cv::Rect> { return this->roi; };
  }
};
