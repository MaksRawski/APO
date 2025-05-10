#pragma once

#include "MaskEditor.hpp"
#include <QMessageBox>
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
  Byte,           // a value in range 0-255 with potentially additional restriction
  Double,         // a double precision value with no restrictions whatsoever
  EnumVariant,    // any variant chosen from a provided list
  ComposableMask, // a mask that is entered via 2 smaller masks
  ChoosableMask,  // same as Mask but with option of choosing a builtin
};

// This weird template struct allows for associating some more data
// with each variant of the Value value
template <DialogValue V> struct ValueTraits;

// ====
// Byte
// ====
struct ByteRestriction {
  uchar low, high;
};
template <> struct ValueTraits<DialogValue::Byte> {
  using Restriction = ByteRestriction;
  using Result = uchar;
};

// ======
// Double
// ======
template <> struct ValueTraits<DialogValue::Double> {
  using Restriction = struct {};
  using Result = double;
};

// =======
// Variant
// =======
struct EnumVariantRestriction {
  std::vector<QString> variants;
  DELETE_DEFAULT_CONSTRUCTORS(EnumVariantRestriction);
};
template <> struct ValueTraits<DialogValue::EnumVariant> {
  using Restriction = EnumVariantRestriction;
  using Result = uint; // index in the combobox
};

// ==============
// ComposableMask
// ==============
template <> struct ValueTraits<DialogValue::ComposableMask> {
  using Restriction = std::vector<uchar>;
  using Result = cv::Mat;
};

// =============
// ChoosableMask
// =============
struct MasksRestriction {
  std::vector<cv::Mat> mats;
  std::vector<QString> descriptions;
  DELETE_DEFAULT_CONSTRUCTORS(MasksRestriction);
};
template <> struct ValueTraits<DialogValue::ChoosableMask> {
  using Restriction = MasksRestriction; // NOTE: this won't be a hard restriction!
  using Result = cv::Mat;
};

template <DialogValue V> struct InputSpec {
  using Restriction = typename ValueTraits<V>::Restriction;
  using ValueType = typename ValueTraits<V>::Result;
  QString name;
  Restriction restriction;
  ValueType initialValue;
  DELETE_DEFAULT_CONSTRUCTORS(InputSpec);
};

QDialogButtonBox *createDialogButtons(QDialog *dialog);
QLineEdit *createValidatedIntEdit(QWidget *parent, int min, int max, int initialValue);
QLineEdit *createValidatedDoubleEdit(QWidget *parent, int initialValue);
QComboBox *createComboBox(std::vector<QString> variants, uint initialIndex);

// Class for declarative creation of dialog windows.
template <DialogValue... Values> class Dialog {
public:
  Dialog() = delete;

  template <DialogValue V> using ResultType = typename ValueTraits<V>::Result;
  using ResultTuple = std::tuple<ResultType<Values>...>;

  Dialog(QWidget *parent, QString title, InputSpec<Values>... inputs) {
    dialog = new QDialog(parent);
    dialog->setWindowTitle(title);

    QFormLayout *form = new QFormLayout(dialog);
    dialog->setLayout(form);
    // create appropriate inputs adding them to the form while returning "accessors"
    accessors = std::make_tuple(createInput(inputs, *form)...);

    form->addRow(createDialogButtons(dialog));
  }

  std::optional<ResultTuple> run() const {
    if (dialog->exec() != QDialog::Accepted)
      return std::nullopt;

    auto optionals =
        std::apply([](auto &&...accessors) { return std::make_tuple(accessors()...); }, accessors);

    bool allOptionalsValid = true;
    std::apply([&](const auto &...opt) { ((allOptionalsValid &= opt.has_value()), ...); },
               optionals);

    if (!allOptionalsValid) {
      QMessageBox::critical(dialog, "Error", "Invalid input.");
      return std::nullopt;
    }

    ResultTuple results =
        std::apply([](const auto &...opt) { return std::make_tuple(opt.value()...); }, optionals);

    return results;
  }

private:
  template <DialogValue V> using Accessor = std::function<std::optional<ResultType<V>>()>;

  QDialog *dialog;
  std::tuple<Accessor<Values>...> accessors;

  Accessor<DialogValue::Byte> //
  createInput(const InputSpec<DialogValue::Byte> &spec, QFormLayout &form) {
    auto *edit = createValidatedIntEdit(dialog, spec.restriction.low, spec.restriction.high,
                                        spec.initialValue);
    form.addRow(spec.name, edit);

    return [edit]() -> std::optional<uchar> {
      bool ok;
      uchar v = static_cast<uchar>(edit->text().toInt(&ok));
      if (ok)
        return v;
      else
        return std::nullopt;
    };
  }

  Accessor<DialogValue::Double> //
  createInput(const InputSpec<DialogValue::Double> &spec, QFormLayout &form) {
    auto *edit = createValidatedDoubleEdit(dialog, spec.initialValue);
    form.addRow(spec.name, edit);

    return [edit]() -> std::optional<double> {
      bool ok;
      double v = static_cast<double>(edit->text().toDouble(&ok));
      if (ok)
        return v;
      else
        return std::nullopt;
    };
  }

  Accessor<DialogValue::EnumVariant> //
  createInput(const InputSpec<DialogValue::EnumVariant> &spec, QFormLayout &form) {
    auto *cb = createComboBox(spec.restriction.variants, spec.initialValue);
    form.addRow(spec.name, cb);

    return [cb]() -> std::optional<uint> {
      int v = cb->currentIndex();
      if (v < 0)
        return std::nullopt;
      else
        return static_cast<uint>(v);
    };
  }

  Accessor<DialogValue::ComposableMask> //
  createInput(const InputSpec<DialogValue::ComposableMask> &spec, QFormLayout &form) {
    cv::Mat mat = spec.initialValue;
    auto mask1 = new MaskEditor(dialog, QSize(mat.cols, mat.rows));
    auto mask2 = new MaskEditor(dialog, QSize(mat.cols, mat.rows));
    mask1->setMask(spec.initialValue);
    mask2->setMask(spec.initialValue);

    auto maskRes = new MaskEditor(dialog, QSize(mat.cols * 2 - 1, mat.rows * 2 - 1));

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
      cv::Mat k2_flipped;
      cv::flip(k2, k2_flipped, -1);

      int pad = k1.rows / 2;
      cv::Mat k1_padded;
      cv::copyMakeBorder(k1, k1_padded, pad, pad, pad, pad, cv::BORDER_CONSTANT, cv::Scalar(0));

      cv::Mat combined;
      cv::filter2D(k1_padded, combined, -1, k2_flipped, cv::Point(-1, -1), 0, cv::BORDER_CONSTANT);
      maskRes->setMask(combined);
    };
    maskChanged();

    QObject::connect(mask1, &MaskEditor::maskChanged, maskChanged);
    QObject::connect(mask2, &MaskEditor::maskChanged, maskChanged);

    return [maskRes]() { return maskRes->getMask(); };
  }

  Accessor<DialogValue::ChoosableMask> //
  createInput(const InputSpec<DialogValue::ChoosableMask> &spec, QFormLayout &form) {
    bool cbVisible = true;
    QComboBox *cb;

    if (spec.restriction.descriptions.size() == 0) {
      cbVisible = false;
    } else {
      cb = createComboBox(spec.restriction.descriptions, 0);
      form.addRow(spec.name, cb);
    }

    cv::Mat mat = spec.initialValue;
    auto mask = new MaskEditor(dialog, QSize(mat.cols, mat.rows));
    mask->setMask(mat);
    form.addRow(mask);

    if (cbVisible) {
      // we update the MaskEditor anytime a ComboBox is changed
      QObject::connect(cb, &QComboBox::currentIndexChanged,
                       [mask, spec](uint index) { mask->setMask(spec.restriction.mats[index]); });
    }

    return [mask]() { return mask->getMask(); };
  }
};
