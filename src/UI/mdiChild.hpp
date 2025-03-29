#pragma once

#include "../imageWrapper.hpp"
#include "imageLabel.hpp"
#include <QLabel>
#include <QScrollArea>
#include <qmdisubwindow.h>
#include <qpixmap.h>
#include <qsize.h>

class MdiChild : public QMdiSubWindow {
  Q_OBJECT
public:
  MdiChild();
  void loadImage(QString filePath);
  void updateImageName(QString name);
  void setImageScale(double zoom);
  const ImageWrapper &getImage() const { return *imageWrapper; };
  const QSize getImageSize() const {
    return QSize(imageWrapper->getWidth(), imageWrapper->getHeight());
  }

signals:
  void imageUpdated(const ImageWrapper &image);

  // public slots:
  // 	void toGrayscale();

private:
  ImageLabel *imageLabel;
  ImageWrapper *imageWrapper;
  QString imageName;
};
