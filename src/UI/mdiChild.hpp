#pragma once

#include "imageLabel.hpp"
#include <QLabel>
#include <qmdisubwindow.h>
#include <qpixmap.h>
#include <QScrollArea>

class MdiChild : public QMdiSubWindow {
	Q_OBJECT
public:
	MdiChild();
	void updatePixmap(QPixmap pixmap);
	QPixmap getPixmap() const;

signals:
	void pixmapUpdated(QPixmap pixmap);

private:
	ImageLabel *imageLabel;
};
