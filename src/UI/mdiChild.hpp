#pragma once

#include "../imageWrapper.hpp"
#include "imageLabel.hpp"
#include <QMdiSubWindow>
#include <QScrollArea>
#include <QTabWidget>
#include <qscrollbar.h>

const QSize CHILD_IMAGE_MARGIN = QSize(30, 70);

class MdiChild : public QMdiSubWindow {
  Q_OBJECT

public:
  explicit MdiChild(ImageWrapper imageWrapper);
  void setImage(const ImageWrapper &image);
  // difference between `swapImage` and `setImage` is that this maintains
  // the same size of the window whereas the previous one adjusts the size to fit the unscaled image
  void swapImage(const ImageWrapper &image);
  void setImageName(QString name);
  void setImageScale(double zoom);

  double getImageScale() const;
  QString getImageName() const { return imageName; }
  QString getImageBasename() const;
  QString getImageNameSuffix() const;
  const ImageWrapper &getImage() const { return imageWrapper; }
  const QSize getImageSize() const {
    return QSize(imageWrapper.getWidth(), imageWrapper.getHeight());
  }
  void emitImageUpdatedSignal() const;

private:
  void updateChannelNames();
  void regenerateChannels();
  QScrollArea &getScrollArea(int index) const;
  ImageLabel &getImageLabel(int index) const;
  const ImageWrapper &getImageWrapper(int index) const;
  void applyDialogFilter(std::optional<std::tuple<cv::Mat, int>> res);

public slots:
  void toRGB();
  void toHSV();
  void toLab();
  void toGrayscale();
  void negate();
  void normalize();
  void equalize();
  void rangeStretch();
  void save();
  void rename();
  void posterize();
  void blurMean();
  void blurMedian();
  void blurGaussian();
  void edgeDetectSobel();
  void edgeDetectLaplacian();
  void edgeDetectCanny();
  void sharpenLaplacian();
  void edgeDetectPrewitt();
  void customMask();
  void customTwoStageFilter();
  void morphologyErode();
  void morphologyDilate();
  void morphologyOpen();
  void morphologyClose();

signals:
  void imageUpdated(const ImageWrapper &image) const;

private slots:
  void tabChanged(int index);

private:
  QTabWidget *tabWidget;
  // entire image
  ImageWrapper imageWrapper;
  QScrollArea *scrollArea;
  ImageLabel *imageLabel;
  // channels
  ImageWrapper imageWrapper1, imageWrapper2, imageWrapper3;
  QScrollArea *scrollArea1, *scrollArea2, *scrollArea3;
  ImageLabel *imageLabel1, *imageLabel2, *imageLabel3;

  double zoom = 1.0;
  QString imageName;
  int tabIndex = 0;
};
