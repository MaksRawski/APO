#include "tab.h"
#include <qlabel.h>
#include <qpixmap.h>
#include <qscrollarea.h>
#include <qtabwidget.h>
#include <qwidget.h>

Tab::Tab(QString name, QPixmap pixmap, QTabWidget *tabWidget) {
  root = new QScrollArea();
  imageLabel = new QLabel(root);

  imageLabel->setAlignment(Qt::AlignCenter);
  root->setWidget(imageLabel);
  root->setAlignment(Qt::AlignCenter);
  root->setVisible(true);

  imageLabel->setPixmap(pixmap);
  imageLabel->adjustSize();

  tabWidget->addTab(root, name);
}
