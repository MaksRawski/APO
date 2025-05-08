#pragma once

#include <opencv2/core/mat.hpp>
#include <qgridlayout.h>
#include <qlineedit.h>
#include <qtmetamacros.h>
#include <qwidget.h>

class MaskEditor : public QWidget {
  Q_OBJECT
public:
  MaskEditor(QDialog *dialog, QSize size);
  QSize getSize() const;
  std::optional<cv::Mat> getMask() const;
  void setMask(const cv::Mat &mat);
  void setReadOnly(bool v);

private:
  QGridLayout *gridLayout;
  QSize size;
  std::vector<std::vector<QLineEdit *>> lineEdits;

signals:
  void maskChanged();
};
