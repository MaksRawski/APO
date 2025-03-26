#pragma once

#include "imageLabel.hpp"
#include <QLabel>
#include <qmdisubwindow.h>
#include <qpixmap.h>
#include <QScrollArea>

enum ImageType {
  Binary,
  GrayScale,
  RGB,
};

class MdiChild : public QMdiSubWindow {
	Q_OBJECT
public:
	MdiChild();
	void updatePixmap(QPixmap pixmap);
	void setImageScale(double zoom);
	QPixmap getPixmap() const;

signals:
	void pixmapUpdated(QPixmap pixmap);

private:
	ImageLabel *imageLabel;
	ImageType imageType;
};
