#pragma once

#include "imageLabel.hpp"
#include <QLabel>
#include <qmdisubwindow.h>
#include <qpixmap.h>
#include <QScrollArea>

enum class ImageType {
  Binary,
  GrayScale,
  RGB,
};

static QString toString(ImageType type) {
  switch (type) {
  case ImageType::Binary:
    return "binary";
  case ImageType::GrayScale:
    return "grayscale";
  case ImageType::RGB:
    return "RGB";
  default:
    return "Unknown";
  }
}

class MdiChild : public QMdiSubWindow {
	Q_OBJECT
public:
	MdiChild();
	void updatePixmap(QPixmap pixmap);
	void updateImageName(QString name);
	void setImageScale(double zoom);
	QPixmap getPixmap() const;

signals:
	void pixmapUpdated(QPixmap pixmap);

public slots:
	void toGrayscale();

private:
	ImageLabel *imageLabel;
	ImageType imageType;
	QString imageName;
};
