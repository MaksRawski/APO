#pragma once

#include "imageLabel.hpp"
#include <QLabel>
#include <qpixmap.h>
#include <QScrollArea>

class MdiChild : public QScrollArea{
	Q_OBJECT
public:
	MdiChild(QPixmap pixmap);
	void updatePixmap(QPixmap pixmap);

signals:
	void pixmapUpdated(QPixmap pixmap);

private:
	ImageLabel *imageLabel;
};
