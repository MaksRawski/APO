#pragma once

#include "imageLabel.hpp"
#include <qmainwindow.h>
#include <qpixmap.h>
#include <qscrollarea.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <QDockWidget>

class Tab {
public:
  Tab(QString name, QPixmap pixmap, QTabWidget *tabWidget);

private:
  QScrollArea *root;
  ImageLabel *imageLabel;
};
