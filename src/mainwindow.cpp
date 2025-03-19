#include "mainwindow.h"
#include "tab.h"
#include <QAction>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QPixmap>
#include <QScrollArea>
#include <qdebug.h>
#include <qfileinfo.h>
#include <qtabwidget.h>
#include <qwidget.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setupMenuBar();

  tabWidget = new QTabWidget(this);
  tabWidget->setTabsClosable(true);
  connect(tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);
  tabs.reserve(3);

  setCentralWidget(tabWidget);
}

void MainWindow::openImage() {
  QString filePath = QFileDialog::getOpenFileName(
      this, tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));

  if (!filePath.isEmpty()) {
    QPixmap pixmap(filePath);

    if (!pixmap.isNull()) {
      // create a tab with that image
      // QWidget *imageTab = createImageTab(pixmap);
      QString fileName = QFileInfo(filePath).fileName();
      Tab *tab = new Tab(fileName, pixmap, tabWidget);
      tabs.push_back(*tab);
    } else {
      // Handle image load error (e.g., show a message box)
      qDebug() << "Failed to load image: " << filePath;
    }
  }
  //
}

void MainWindow::setupMenuBar() {
  QMenu *fileMenu = menuBar()->addMenu("&File");
  QAction *openAction = fileMenu->addAction("&Open...");
  // QMenu *imageMenu = menuBar()->addMenu("&Image");
  // QAction *histAction = imageMenu->addAction("&Histogram");

  connect(openAction, &QAction::triggered, this, &MainWindow::openImage);
}

void MainWindow::closeTab(int index) {
    tabWidget->removeTab(index);
}
