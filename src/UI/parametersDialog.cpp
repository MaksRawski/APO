#include "parametersDialog.hpp"

#include <QFormLayout>
#include <QIntValidator>
#include <QLineEdit>
#include <QMessageBox>
#include <QString>
#include <qcombobox.h>
#include <qdialogbuttonbox.h>
#include <QComboBox>
#include <qvalidator.h>

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
  formLayout->addRow("p1 (0-255):", p1LineEdit);
  formLayout->addRow("p2 (0-255):", p2LineEdit);
  formLayout->addRow("q3 (0-255):", q3LineEdit);
  formLayout->addRow("q4 (0-255):", q4LineEdit);

  QDialogButtonBox *buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                           &dialog); // Parent to the dialog
  QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog,
                   &QDialog::accept);
  QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog,
                   &QDialog::reject);
  formLayout->addRow(buttonBox);

  int result = dialog.exec();

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
  dialog.setWindowTitle("Enter number of ");

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

std::optional<uchar> kernelSizeDialog(QWidget *parent, uchar i,
									  std::vector<uchar> allowed_sizes) {
  QDialog dialog(parent);
  dialog.setWindowTitle("Select kernel size");

  QFormLayout *formLayout = new QFormLayout(&dialog);
  QComboBox cb;
  for (auto v : allowed_sizes) {
    cb.addItem(QString("%1x%1").arg(v), v);
  }
  cb.setCurrentIndex(i);
  formLayout->addRow("K", &cb);

  QDialogButtonBox *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  formLayout->addRow(buttonBox);

  int result = dialog.exec();

  if (result == QDialog::Accepted) {
    if (cb.currentIndex() > -1) {
      return allowed_sizes[cb.currentIndex()];
    } else {
      QMessageBox::critical(
          nullptr, "Error",
          "Invalid input. Please select a kernel size.");
      return std::nullopt;
    }
  } else {
    return std::nullopt;
  }
}

std::optional<std::tuple<uchar, double>>
gaussianBlurDialog(QWidget *parent, uchar i, std::vector<uchar> allowed_sizes) {
  QDialog dialog(parent);
  dialog.setWindowTitle("Select kernel size and sigma");

  QFormLayout *formLayout = new QFormLayout(&dialog);
  QComboBox cb;
  for (auto v : allowed_sizes) {
    cb.addItem(QString("%1x%1").arg(v), v);
  }
  cb.setCurrentIndex(i);
  formLayout->addRow("K", &cb);

  QLineEdit stdDevLineEdit;
  QDoubleValidator validator(&dialog);
  stdDevLineEdit.setValidator(&validator);
  stdDevLineEdit.setText("1.0");
  formLayout->addRow("std dev", &stdDevLineEdit);

  QDialogButtonBox *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  formLayout->addRow(buttonBox);

  int result = dialog.exec();

  if (result == QDialog::Accepted) {
    if (cb.currentIndex() > -1) {
		double stdDev = stdDevLineEdit.text().toDouble();
      return std::tuple(allowed_sizes[cb.currentIndex()], stdDev);
    } else {
      QMessageBox::critical(
          nullptr, "Error",
          "Invalid input. Please select a kernel size.");
      return std::nullopt;
    }
  } else {
    return std::nullopt;
  }
}
