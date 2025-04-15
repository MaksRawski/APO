#include "parametersDialog.hpp"

#include <QComboBox>
#include <QFormLayout>
#include <QIntValidator>
#include <QLineEdit>
#include <QMessageBox>
#include <QString>
#include <qcombobox.h>
#include <qdialogbuttonbox.h>
#include <qvalidator.h>

#include "parametersDialog.hpp"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleValidator>
#include <QFormLayout>
#include <QIntValidator>
#include <QLineEdit>
#include <QMessageBox>
#include <QString>
#include <optional>
#include <tuple>
#include <vector>

namespace {
QLineEdit *createValidatedLineEdit(QWidget *parent, int min, int max,
                                   int initialValue) {
  QLineEdit *lineEdit = new QLineEdit(parent);
  QIntValidator *validator = new QIntValidator(min, max, parent);
  lineEdit->setValidator(validator);
  lineEdit->setText(QString::number(initialValue));
  return lineEdit;
}

QDialogButtonBox *createDialogButtons(QDialog *dialog) {
  auto *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
  QObject::connect(buttonBox, &QDialogButtonBox::accepted, dialog,
                   &QDialog::accept);
  QObject::connect(buttonBox, &QDialogButtonBox::rejected, dialog,
                   &QDialog::reject);
  return buttonBox;
}
} // namespace

std::optional<std::tuple<uchar, uchar, uchar, uchar>>
rangeStretchDialog(QWidget *parent, uchar initialP1, uchar initialP2,
                   uchar initialQ3, uchar initialQ4) {
  QDialog dialog(parent);
  dialog.setWindowTitle("Select ranges");

  QFormLayout formLayout(&dialog);
  auto *p1Edit = createValidatedLineEdit(&dialog, 0, 255, initialP1);
  auto *p2Edit = createValidatedLineEdit(&dialog, 0, 255, initialP2);
  auto *q3Edit = createValidatedLineEdit(&dialog, 0, 255, initialQ3);
  auto *q4Edit = createValidatedLineEdit(&dialog, 0, 255, initialQ4);

  formLayout.addRow("p1 (0-255):", p1Edit);
  formLayout.addRow("p2 (0-255):", p2Edit);
  formLayout.addRow("q3 (0-255):", q3Edit);
  formLayout.addRow("q4 (0-255):", q4Edit);
  formLayout.addRow(createDialogButtons(&dialog));

  if (dialog.exec() == QDialog::Accepted) {
    bool ok1, ok2, ok3, ok4;
    uchar p1 = static_cast<uchar>(p1Edit->text().toInt(&ok1));
    uchar p2 = static_cast<uchar>(p2Edit->text().toInt(&ok2));
    uchar q3 = static_cast<uchar>(q3Edit->text().toInt(&ok3));
    uchar q4 = static_cast<uchar>(q4Edit->text().toInt(&ok4));
    if (ok1 && ok2 && ok3 && ok4)
      return std::make_tuple(p1, p2, q3, q4);
    QMessageBox::critical(
        nullptr, "Error",
        "Invalid input. Please enter integers between 0 and 255.");
  }

  return std::nullopt;
}

std::optional<uchar> posterizeDialog(QWidget *parent, uchar N) {
  QDialog dialog(parent);
  dialog.setWindowTitle("Enter number of levels");

  QFormLayout formLayout(&dialog);
  auto *edit = createValidatedLineEdit(&dialog, 1, 255, N);
  formLayout.addRow("N", edit);
  formLayout.addRow(createDialogButtons(&dialog));

  if (dialog.exec() == QDialog::Accepted) {
    bool ok;
    uchar n = static_cast<uchar>(edit->text().toInt(&ok));
    if (!ok) {
      QMessageBox::critical(
          parent, "Error",
          "Invalid input. Please enter a number between 1 and 255.");
      return std::nullopt;
    }
    return n;
  }

  return std::nullopt;
}

std::optional<uchar> kernelSizeDialog(QWidget *parent) {
  QDialog dialog(parent);
  dialog.setWindowTitle("Select kernel size");

  QFormLayout formLayout(&dialog);
  QComboBox comboBox;
  std::vector<uchar> sizes{3, 5, 7};
  for (uchar size : sizes)
    comboBox.addItem(QString("%1x%1").arg(size), size);
  comboBox.setCurrentIndex(0);

  formLayout.addRow("K", &comboBox);
  formLayout.addRow(createDialogButtons(&dialog));

  if (dialog.exec() == QDialog::Accepted && comboBox.currentIndex() >= 0) {
    return sizes[comboBox.currentIndex()];
  }

  QMessageBox::critical(parent, "Error",
                        "Invalid input. Please select a kernel size.");
  return std::nullopt;
}

std::optional<std::tuple<uchar, double>> gaussianBlurDialog(QWidget *parent) {
  QDialog dialog(parent);
  dialog.setWindowTitle("Select kernel size and sigma");

  QFormLayout formLayout(&dialog);
  QComboBox comboBox;
  std::vector<uchar> sizes{3, 5, 7};
  for (uchar size : sizes)
    comboBox.addItem(QString("%1x%1").arg(size), size);
  comboBox.setCurrentIndex(0);

  QLineEdit sigmaEdit;
  QDoubleValidator doubleValidator(0.0, 100.0, 2, &dialog);
  sigmaEdit.setValidator(&doubleValidator);
  sigmaEdit.setText("1.0");

  formLayout.addRow("K", &comboBox);
  formLayout.addRow("σ (std dev)", &sigmaEdit);
  formLayout.addRow(createDialogButtons(&dialog));

  if (dialog.exec() == QDialog::Accepted) {
    bool ok;
    double sigma = sigmaEdit.text().toDouble(&ok);
    if (comboBox.currentIndex() >= 0 && ok) {
      return std::make_tuple(sizes[comboBox.currentIndex()], sigma);
    }
    QMessageBox::critical(nullptr, "Error",
                          "Invalid input. Please enter a valid sigma.");
  }

  return std::nullopt;
}

std::optional<std::tuple<uchar, Direction>> sobelDialog(QWidget *parent) {
  QDialog dialog(parent);
  dialog.setWindowTitle("Select kernel size and direction");

  QFormLayout formLayout(&dialog);
  QComboBox cbKernelSize;
  std::vector<uchar> sizes{3, 5, 7};
  for (uchar size : sizes)
    cbKernelSize.addItem(QString("%1x%1").arg(size), size);
  cbKernelSize.setCurrentIndex(0);
  QComboBox cbDir;
  cbDir.addItem(QString("Horizontal"));
  cbDir.addItem(QString("Vertical"));
  cbDir.setCurrentIndex(0);

  formLayout.addRow("K", &cbKernelSize);
  formLayout.addRow("Direction", &cbDir);
  formLayout.addRow(createDialogButtons(&dialog));

  if (dialog.exec() == QDialog::Accepted) {
    if (cbKernelSize.currentIndex() >= 0 && cbDir.currentIndex() >= 0) {
      Direction dir = cbDir.currentIndex() == 0 ? Direction::Horizontal
                                                : Direction::Vertical;
      return std::make_tuple(sizes[cbKernelSize.currentIndex()], dir);
    }
    QMessageBox::critical(nullptr, "Error", "Invalid input.");
  }

  return std::nullopt;
}

std::optional<std::tuple<uchar, uchar, uchar>> cannyDialog(QWidget *parent) {
  QDialog dialog(parent);
  dialog.setWindowTitle("Select threshold");

  QFormLayout formLayout(&dialog);
  auto *startLineEdit = createValidatedLineEdit(&dialog, 0, 255, 100);
  auto *endLineEdit = createValidatedLineEdit(&dialog, 0, 255, 200);
  QComboBox comboBox;
  std::vector<uchar> sizes{1, 3, 5, 7};
  for (uchar size : sizes)
    comboBox.addItem(QString("%1x%1").arg(size), size);
  comboBox.setCurrentIndex(1);

  formLayout.addRow("Start (0-255):", startLineEdit);
  formLayout.addRow("End (0-255):", endLineEdit);
  formLayout.addRow(createDialogButtons(&dialog));

  if (dialog.exec() == QDialog::Accepted) {
    bool ok1, ok2;
    uchar start = static_cast<uchar>(startLineEdit->text().toInt(&ok1));
    uchar end = static_cast<uchar>(endLineEdit->text().toInt(&ok2));
    if (ok1 && ok2 && comboBox.currentIndex() >= 0) {
      uchar k = sizes[comboBox.currentIndex()];
      return std::make_tuple(k, start, end);
    }
    QMessageBox::critical(
        parent, "Error",
        "Invalid input. Please enter integers between 0 and 255.");
  }

  return std::nullopt;
}
