#pragma once

#include "MaskEditor.hpp"
#include <QMessageBox>
#include <opencv2/core/mat.hpp>
#include <optional>
#include <qcombobox.h>
#include <qdialog.h>
#include <qdialogbuttonbox.h>
#include <qformlayout.h>
#include <qlineedit.h>
#include <qtypes.h>
#include <qvalidator.h>
#include <tuple>
#include <vector>

// Goal of each dialog is to get some values from the user.
// This enum holds all possible abstract types of values that a dialog
// may want to get from the user.
enum class DialogValue {
  Byte,          // a value in range 0-255 with potentially additional restriction
  Double,        // a double precision value with no restrictions whatsoever
  EnumVariant,   // any variant chosen from a provided list
  Mask,          // a matrix of given size
  ChoosableMask, // same as Mask but with option of choosing a builtin
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
};
template <> struct ValueTraits<DialogValue::EnumVariant> {
  using Restriction = EnumVariantRestriction;
  using Result = uint; // index in the combobox
};

// ====
// Mask
// ====
template <> struct ValueTraits<DialogValue::Mask> {
  using Restriction = QSize;
  using Result = cv::Mat;
};

// =============
// ChoosableMask
// =============
struct MasksRestriction {
  std::vector<cv::Mat> mats;
  std::vector<QString> descriptions;
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
};

QDialogButtonBox *createDialogButtons(QDialog *dialog);
QLineEdit *createValidatedIntEdit(QWidget *parent, int min, int max, int initialValue);
QLineEdit *createValidatedDoubleEdit(QWidget *parent, int initialValue);
QComboBox *createComboBox(QWidget *parent, std::vector<QString> variants, uint initialIndex);

// Class for declarative creation of dialog windows.
template <DialogValue... Values> class Dialog {
public:
  Dialog() = delete;

  template <DialogValue V> using ResultType = typename ValueTraits<V>::Result;
  using ResultTuple = std::tuple<ResultType<Values>...>;

  Dialog(QWidget *parent, QString title, InputSpec<Values>... inputs) {
    dialog = new QDialog(parent);
    dialog->setWindowTitle(title);

    QFormLayout form(dialog);
    // create appropriate inputs adding them to the form while returning "accessors"
    accessors = std::make_tuple(createInput(inputs, form)...);

    form.addRow(createDialogButtons(dialog));
  }

  std::optional<ResultTuple> run() const {
    if (dialog->exec() != QDialog::Accepted)
      return std::nullopt;

    auto optionals =
        std::apply([](auto &&...accessors) { return std::make_tuple(accessors()...); }, accessors);

    bool allOptionalsValid = true;
    std::apply([&](const auto &...opt) { ((allOptionalsValid &= opt.has_value()), ...); }, optionals);

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
  // NOTE: Since MaskEditor is not a QWidget it's therefore not managed by Qt.
  // So to avoid memory leaks, all of its instances have to be tied to this class so that they can
  // be dropped once the dialog is closed.
  std::vector<MaskEditor> maskEditors;

  Accessor<DialogValue::Byte> //
  createInput(const InputSpec<DialogValue::Byte> &spec, QFormLayout &form) {
    auto *edit = createValidatedIntEdit(dialog, spec.restriction.low, spec.restriction.high,
                                        spec.initialValue);
    form.addRow(spec.name, edit);

    return [&]() -> std::optional<uchar> {
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

    return [&]() -> std::optional<double> {
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
    auto *cb = createComboBox(dialog, spec.restriction.variants, spec.initialValue);
    form.addRow(spec.name, cb);

    return [&]() -> std::optional<uint> {
      int v = cb->currentIndex();
      if (v < 0)
        return std::nullopt;
      else
        return static_cast<uint>(v);
    };
  }

  Accessor<DialogValue::Mask> //
  createInput(const InputSpec<DialogValue::Mask> &spec, QFormLayout &form) {
    cv::Mat mat = spec.initialValue;
    auto mask = MaskEditor(dialog, QSize(mat.cols, mat.rows));
    form.addItem(&mask.getGrid());

    // for the accessor to be able to access the current mask we must provide it an index to
    // maskEditors at which this mask is going to be stored
    uint i = maskEditors.size();
    maskEditors.push_back(mask);

    return [this, i]() { return maskEditors[i].getMask(); };
  }

  Accessor<DialogValue::ChoosableMask> //
  createInput(const InputSpec<DialogValue::ChoosableMask> &spec, QFormLayout &form) {
    auto *cb = createComboBox(dialog, spec.restriction.descriptions, 0);
    form.addRow(spec.name, cb);

    cv::Mat mat = spec.initialValue;
    auto mask = MaskEditor(dialog, QSize(mat.cols, mat.rows));
    form.addItem(&mask.getGrid());

    // we update the MaskEditor anytime a ComboBox is changed
    QObject::connect(cb, &QComboBox::currentIndexChanged,
                     [&](uint index) { mask.setMask(spec.restriction.mats[index]); });

    // for the accessor to be able to access the current mask we must provide it an index to
    // maskEditors at which this mask is going to be stored
    uint i = maskEditors.size();
    maskEditors.push_back(mask);

    return [this, i]() { return maskEditors[i].getMask(); };
  }
};
