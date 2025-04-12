#pragma once

#include "../imageWrapper.hpp"
#include "imageLabel.hpp"
#include <QLabel>
#include <QScrollArea>
#include <qmdisubwindow.h>
#include <qpixmap.h>
#include <qscrollarea.h>
#include <qsize.h>
#include <qtabwidget.h>

const QSize CHILD_IMAGE_MARGIN = QSize(30, 70);

class MdiChild : public QMdiSubWindow {
  Q_OBJECT
public:
  MdiChild();
  void loadImage(QString filePath);
  void setImage(const QPixmap &pixmap);
  void setImage(const ImageWrapper &image);
  // difference between this and `setImage` is that this maintains
  // the same size of the window whereas the previous one adjusts the size to fit the unscaled image
  void swapImage(const ImageWrapper &image);
  void swapImage(const QPixmap &image);
  void setImageName(QString name);
  void setImageScale(double zoom);

  double getImageScale() const { return zoom; }
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
  void toLab();
  void toRGB();
  void toHSV();
  void toGrayscale();
  void negate();
  void normalize();
  void equalize();

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
