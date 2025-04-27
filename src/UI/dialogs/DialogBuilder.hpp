#pragma once

#include "MaskEditor.hpp"
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
#include <QMessageBox>

// Goal of each dialog is to get some values from the user.
// This enum holds all possible abstract types of values that a dialog
// may want to get from the user.
enum class DialogValue {
  Byte,
  Double,
  EnumVariant,
  Mask,
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

// ====
// Double
// ====
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
  using Restriction = struct {};
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
    accessors = std::make_tuple((createInput(std::get<Values>(inputs), &form), ...));

    form.addRow(createDialogButtons(dialog));
  }

  std::optional<ResultTuple> run() const {
    if (dialog->exec() != QDialog::Accepted)
      return std::nullopt;
    auto values = (std::get<Accessor<Values>>(accessors)(), ...);

    bool hasNullOpt = false;
    ((hasNullOpt = hasNullOpt || !(std::get<Values>(values).has_value())), ...);
    if (hasNullOpt) {
      QMessageBox::critical(dialog, "Error", "Invalid input.");
      return std::nullopt;
    }

    return (std::get<Values>(values).value(), ...);
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

    return [&]() {
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

    return [&]() {
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

    return [&]() {
      int v = cb->currentIndex();
      if (v < 0)
        return std::nullopt;
      else
        return v;
    };
  }

  Accessor<DialogValue::Mask> //
  createInput(const InputSpec<DialogValue::Mask> &spec, QFormLayout &form) {
    cv::Mat mat = spec.initialValue;
    // HACK: constructing MaskEditor on heap so that it outlives the accessor
    // this ends up in a memory leak!
    auto *mask = new MaskEditor(dialog, QSize(mat.cols, mat.rows));
    form.addItem(&mask->getGrid());
    return [&]() { return mask->getMask(); };
  }
};
