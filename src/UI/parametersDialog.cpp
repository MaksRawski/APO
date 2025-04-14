#include "parametersDialog.hpp"

#include <QFormLayout>
#include <QIntValidator>
#include <QLineEdit>
#include <QMessageBox>
#include <QString>
#include <qdialogbuttonbox.h>

std::optional<std::tuple<uchar, uchar, uchar, uchar>>
rangeStretchDialog(QWidget *parent, uchar initialP1, uchar initialP2,
				   uchar initialQ3, uchar initialQ4) {
  QDialog dialog(parent);
  dialog.setWindowTitle("Enter Parameters");

  // Create a QFormLayout.
  QFormLayout *formLayout = new QFormLayout(&dialog);

  // Create QLineEdit widgets.
  QLineEdit *p1LineEdit = new QLineEdit(&dialog);
  QLineEdit *p2LineEdit = new QLineEdit(&dialog);
  QLineEdit *q3LineEdit = new QLineEdit(&dialog);
  QLineEdit *q4LineEdit = new QLineEdit(&dialog);

  // Create and set the validator.
  QIntValidator *validator = new QIntValidator(0, 255, &dialog);
  p1LineEdit->setValidator(validator);
  p2LineEdit->setValidator(validator);
  q3LineEdit->setValidator(validator);
  q4LineEdit->setValidator(validator);

  // Set initial values.
  p1LineEdit->setText(QString::number(initialP1));
  p2LineEdit->setText(QString::number(initialP2));
  q3LineEdit->setText(QString::number(initialQ3));
  q4LineEdit->setText(QString::number(initialQ4));

  // Add widgets to the form layout.
  formLayout->addRow("Parameter p1 (0-255):", p1LineEdit);
  formLayout->addRow("Parameter p2 (0-255):", p2LineEdit);
  formLayout->addRow("Parameter q3 (0-255):", q3LineEdit);
  formLayout->addRow("Parameter q4 (0-255):", q4LineEdit);

  QDialogButtonBox *buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                           &dialog); // Parent to the dialog
  QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog,
                   &QDialog::accept);
  QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog,
                   &QDialog::reject);
  formLayout->addRow(buttonBox);

  uchar result = dialog.exec();

  if (result == QDialog::Accepted) {
    bool ok1 = false, ok2 = false, ok3 = false, ok4 = false;
    // Get the values from the QLineEdits.
    uchar p1 = static_cast<uchar>(p1LineEdit->text().toInt(&ok1));
    uchar p2 = static_cast<uchar>(p2LineEdit->text().toInt(&ok2));
    uchar q3 = static_cast<uchar>(q3LineEdit->text().toInt(&ok3));
    uchar q4 = static_cast<uchar>(q4LineEdit->text().toInt(&ok4));

    if (ok1 && ok2 && ok3 && ok4) {
      return std::make_tuple(p1, p2, q3, q4);
    } else {
      QMessageBox::critical(
          nullptr, "Error",
          "Invalid input. Please enter integers between 0 and 255.");
      return std::nullopt;
    }
  } else {
    return std::nullopt;
  }
}

std::optional<uchar> posterizeDialog(QWidget *parent, uchar N) {
  QDialog dialog(parent);
  dialog.setWindowTitle("Posterize parameters");

  QFormLayout *formLayout = new QFormLayout(&dialog);
  QLineEdit *lineEdit = new QLineEdit(&dialog);
  QIntValidator *validator = new QIntValidator(1, 255, &dialog);
  lineEdit->setValidator(validator);
  lineEdit->setText(QString::number(N));
  formLayout->addRow("N", lineEdit);

  QDialogButtonBox *buttonBox =
	  new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  formLayout->addRow(buttonBox);

  int result = dialog.exec();

  if (result == QDialog::Accepted) {
    bool ok;
    uchar n = static_cast<uchar>(lineEdit->text().toInt(&ok));
	if (ok) {
	  return n;
	} else {
	  QMessageBox::critical(
		  nullptr, "Error",
		  "Invalid input. Please enter integers between 1 and 255.");
	  return std::nullopt;
	}

  } else {
    return std::nullopt;
  }
}
