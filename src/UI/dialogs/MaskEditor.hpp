#pragma once

#include <opencv2/core/mat.hpp>
#include <qgridlayout.h>
#include <qlineedit.h>

class MaskEditor {
public:
  MaskEditor(QDialog *dialog, QSize size);
  QSize getSize() const;
  QGridLayout& getGrid() const;
  void setMask(const cv::Mat &mat);
  std::optional<cv::Mat> getMask() const;

private:
  QGridLayout *gridLayout;
  QSize size;
  std::vector<std::vector<QLineEdit *>> lineEdits;
};
