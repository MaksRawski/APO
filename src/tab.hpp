#pragma once

#include "imageLabel.hpp"
#include <qpixmap.h>
#include <qscrollarea.h>
#include <qtabwidget.h>

class Tab {
public:
  Tab(QString name, QPixmap pixmap, QTabWidget *tabWidget) {
    root = new QScrollArea();
    imageLabel = new ImageLabel(root);

    imageLabel->setImage(pixmap);

    root->setWidget(imageLabel);
    root->setWidgetResizable(true);

    tabWidget->addTab(root, name);
  }

private:
  QScrollArea *root;
  ImageLabel *imageLabel;
};
