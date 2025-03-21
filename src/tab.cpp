#include "tab.hpp"
#include <qwidget.h>

Tab::Tab(QString name, QPixmap pixmap, QTabWidget *tabWidget) {
  root = new QScrollArea();
  imageLabel = new ImageLabel(root);

  // load the image
  imageLabel->setImage(pixmap);

  // display imageLabel
  root->setWidget(imageLabel);
  root->setWidgetResizable(true);

  tabWidget->addTab(root, name);
}
