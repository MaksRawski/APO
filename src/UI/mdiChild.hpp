#pragma once

#include "../imageWrapper.hpp"
#include "imageLabel.hpp"
#include <QLabel>
#include <QScrollArea>
#include <qmdisubwindow.h>
#include <qpixmap.h>
#include <qsize.h>
#include <qtabwidget.h>

class MdiChild : public QMdiSubWindow {
  Q_OBJECT
public:
  MdiChild();
  void loadImage(QString filePath);
  void updateImageName(QString name);
  void setImageScale(double zoom);
  void updateChannelNames();
  const ImageWrapper &getImage() const { return *imageWrapper; };
  const QSize getImageSize() const {
    return QSize(imageWrapper->getWidth(), imageWrapper->getHeight());
  }

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
  ImageLabel *imageLabel1, *imageLabel2, *imageLabel3;

  QString imageName;
  int prevTabIndex = 0;
};
