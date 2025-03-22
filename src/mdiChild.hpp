#pragma once

#include "imageLabel.hpp"
#include <QLabel>
#include <qpixmap.h>
#include <QScrollArea>

class MdiChild : public QScrollArea{
	Q_OBJECT
public:
	MdiChild(QPixmap pixmap);

private:
	ImageLabel *imageLabel;
};
