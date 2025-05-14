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

  QDoubleValidator *validator = new QDoubleValidator(-100'000.0, 100'000.0, 8, dialog);

  if (size.width() < 1 || size.height() < 1)
    throw new std::invalid_argument("MaskEditor's size must be positive!");

  this->size = size;

  for (int row = 0; row < size.height(); ++row) {
    std::vector<QLineEdit *> lineEditsRow;
    for (int col = 0; col < size.width(); ++col) {
      QLineEdit *edit = new QLineEdit("0.0", dialog);
      QObject::connect(edit, &QLineEdit::textChanged, [this]() { emit maskChanged(); });
      edit->setFixedWidth(50);
      edit->setValidator(validator);
      gridLayout->addWidget(edit, row, col);
      lineEditsRow.push_back(edit);
    }
    lineEdits.push_back(lineEditsRow);
  }
}

QSize MaskEditor::getSize() const { return size; }

void MaskEditor::setMask(const cv::Mat &mat) {
  if (mat.rows != size.height() || mat.cols != size.width()) {
    throw new std::invalid_argument(
        "Tried to set a mask that's of a different size than the MaskEditor!");
  }

  for (int row = 0; row < mat.rows; ++row) {
    // TODO: suuport raw bytes
    auto *rowPtr = mat.ptr<double>(row);
    for (int col = 0; col < mat.cols; ++col) {
      lineEdits[row][col]->setText(QString::number(rowPtr[col]));
    }
  }
}

void MaskEditor::setReadOnly(bool v) {
  for (int row = 0; row < size.height(); ++row) {
    for (int col = 0; col < size.width(); ++col) {
      lineEdits[row][col]->setReadOnly(v);
    }
  }
}

std::optional<cv::Mat> MaskEditor::getMask() const {
  cv::Mat mask(size.height(), size.width(), CV_64FC1);

  for (int row = 0; row < size.height(); ++row) {
    auto *maskRowPtr = mask.ptr<double>(row);
    for (int col = 0; col < size.width(); ++col) {
      bool conversionOk;
      double val = lineEdits[row][col]->text().toDouble(&conversionOk);
      if (!conversionOk)
        return std::nullopt;
      maskRowPtr[col] = val;
    }
  }
  return mask;
}
