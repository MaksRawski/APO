#pragma once

#include <qlabel.h>
#include <qpixmap.h>
#include <qscrollarea.h>
#include <qtabwidget.h>

class Tab {
public:
  Tab(QString name, QPixmap pixmap, QTabWidget *tabWidget);

private:
  QPixmap *image;
  QScrollArea *root;
  QLabel *imageLabel;
};
