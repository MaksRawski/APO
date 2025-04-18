#pragma once

#include "../imageWrapper.hpp"
#include "imageLabel.hpp"
#include <QScrollArea>
#include <QMdiSubWindow>
#include <QTabWidget>

const QSize CHILD_IMAGE_MARGIN = QSize(30, 70);

class MdiChild : public QMdiSubWindow {
  Q_OBJECT
public:
  MdiChild();
  void loadImage(QString filePath);
  void setImage(const QPixmap &pixmap);
  void setImage(const ImageWrapper &image);
  // difference between `swapImage` and `setImage` is that this maintains
  // the same size of the window whereas the previous one adjusts the size to fit the unscaled image
  void swapImage(const ImageWrapper &image);
  void swapImage(const cv::Mat &image);
  void swapImage(const QPixmap &image);
  void setImageName(QString name);
  void setImageScale(double zoom);

  double getImageScale() const { return imageLabel->getImageScale(); }
  QString getImageName() const { return imageName; }
  QString getImageBasename() const;
  QString getImageNameSuffix() const;
  const ImageWrapper &getImage() const { return *imageWrapper; };
  const QSize getImageSize() const {
    return QSize(imageWrapper->getWidth(), imageWrapper->getHeight());
  }

private:
  void updateChannelNames();
  void regenerateChannels();

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

signals:
  void imageUpdated(const ImageWrapper &image);

private slots:
  void tabChanged(int index);

private:
  QTabWidget *tabWidget;
  ImageLabel *imageLabel;
  ImageWrapper *imageWrapper;
  // image channels
  ImageWrapper imageWrapper1, imageWrapper2, imageWrapper3;
  QScrollArea *scrollArea1, *scrollArea2, *scrollArea3;
  ImageLabel *imageLabel1, *imageLabel2, *imageLabel3;

  double zoom = 1.0;
  QString imageName;
  int prevTabIndex = 0;
};
