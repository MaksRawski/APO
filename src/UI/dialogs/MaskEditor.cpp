#include "MaskEditor.hpp"
#include <QComboBox>
#include <QFormLayout>
#include <QIntValidator>
#include <QLineEdit>
#include <QMessageBox>
#include <QString>
#include <opencv2/core/base.hpp>
#include <qcombobox.h>
#include <qgridlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlogging.h>
#include <qobject.h>
#include <qwidget.h>
#include <stdexcept>

MaskEditor::MaskEditor(QDialog *dialog, QSize size) : QWidget(dialog) {
  gridLayout = new QGridLayout;
  setLayout(gridLayout);

  if (size.width() < 1 || size.height() < 1)
    throw new std::invalid_argument("MaskEditor's size must be positive!");

  this->size = size;

  for (int row = 0; row < size.height(); ++row) {
    std::vector<QSpinBox *> spinboxRow;
    for (int col = 0; col < size.width(); ++col) {
      QSpinBox *spinbox = new QSpinBox(dialog);
      spinbox->setRange(-100, 100);
      QObject::connect(spinbox, &QSpinBox::textChanged, [this]() { emit maskChanged(); });
      spinbox->setFixedWidth(50);
      gridLayout->addWidget(spinbox, row, col);
      spinboxRow.push_back(spinbox);
    }
    spinboxes.push_back(spinboxRow);
  }
}

QSize MaskEditor::getSize() const { return size; }

void MaskEditor::setMask(const cv::Mat &mat) {
  if (mat.rows != size.height() || mat.cols != size.width()) {
    throw new std::invalid_argument(
        "Tried to set a mask that's of a different size than the MaskEditor!");
  }

  for (int row = 0; row < mat.rows; ++row) {
    auto *rowPtr = mat.ptr<char>(row);
    for (int col = 0; col < mat.cols; ++col) {
      spinboxes[row][col]->setValue(rowPtr[col]);
    }
  }
}

void MaskEditor::setReadOnly(bool v) {
  for (int row = 0; row < size.height(); ++row) {
    for (int col = 0; col < size.width(); ++col) {
      spinboxes[row][col]->setReadOnly(v);
    }
  }
}

std::optional<cv::Mat> MaskEditor::getMask() const {
  cv::Mat mask(size.height(), size.width(), CV_8SC1);

  for (int row = 0; row < size.height(); ++row) {
    auto *maskRowPtr = mask.ptr<char>(row);
    for (int col = 0; col < size.width(); ++col) {
      signed char val = spinboxes[row][col]->value();
      maskRowPtr[col] = val;
    }
  }
  return mask;
}
