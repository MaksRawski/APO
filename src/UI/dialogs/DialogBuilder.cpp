#include "DialogBuilder.hpp"
#include <qvalidator.h>

QDialogButtonBox *createDialogButtons(QDialog *dialog) {
  auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  QObject::connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
  QObject::connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
  return buttonBox;
}

QSpinBox *createValidatedIntEdit(QWidget *parent, int min, int max, int initialValue) {
  QSpinBox *spinbox = new QSpinBox(parent);
  spinbox->setValue(initialValue);
  spinbox->setRange(min, max);
  return spinbox;
}

QLineEdit *createValidatedDoubleEdit(QWidget *parent, int initialValue) {
  QLineEdit *lineEdit = new QLineEdit;
  QDoubleValidator *validator = new QDoubleValidator(parent);
  lineEdit->setValidator(validator);
  lineEdit->setText(QString::number(initialValue));
  return lineEdit;
}

QComboBox *createComboBox(std::vector<QString> variants, uint initialIndex) {
  QComboBox *cb = new QComboBox;
  for (auto v : variants)
    cb->addItem(v);
  cb->setCurrentIndex(initialIndex);
  return cb;
}
