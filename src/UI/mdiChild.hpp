#pragma once

#include "../imageWrapper.hpp"
#include "ImageViewer.hpp"
#include "dialogs/utils.hpp"
#include <QMdiSubWindow>
#include <QScrollArea>
#include <QTabWidget>
#include <qscrollbar.h>

class MdiChild : public QMdiSubWindow {
  Q_OBJECT

public:
  explicit MdiChild(ImageWrapper imageWrapper);
  void setImage(const ImageWrapper &image);
  // difference between `swapImage` and `setImage` is that this maintains
  // the same size of the window whereas the previous one adjusts the size to fit the unscaled image
  void swapImage(const ImageWrapper &image);
  void trySwapImage(const std::optional<ImageWrapper> &image);
  void setImageName(QString name);
  void fitImage();

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
  ImageViewer &getImageViewer(int index) const;
  const ImageWrapper &getImageWrapper(int index) const;
  void ask4maskAndApply(const std::vector<cv::Mat> &mats, const std::vector<QString> &names);
  void ask4structuringElementAndApply(
      std::function<std::optional<cv::Mat>(StructuringElement::ValueType, BorderTypes::ValueType)> fn);

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
  void morphologySkeletonize();
  void houghTransform();
  void thresholdManual();
  void thresholdAdaptive();
  void thresholdOtsu();
  void profileLine();
  void affineTransform();

signals:
  void imageUpdated(const ImageWrapper &image) const;

private slots:
  void tabChanged(int index);

private:
  QTabWidget *tabWidget;
  // entire image
  ImageWrapper imageWrapper;
  ImageViewer *mainImage;
  // channels
  ImageWrapper imageWrapper1, imageWrapper2, imageWrapper3;
  ImageViewer *image1, *image2, *image3;

  QString imageName;
  int tabIndex = 0;
};
