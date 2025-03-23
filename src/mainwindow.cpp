#include "mainwindow.hpp"
#include "histogramWidget.hpp"
#include "mdiChild.hpp"
#include <QAction>
#include <QFileDialog>
#include <QLabel>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPixmap>
#include <QTextEdit>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setupMenuBar();
  setupUI();
}

void MainWindow::setupMenuBar() {
  QMenu *fileMenu = menuBar()->addMenu("&File");
  QAction *openAction = fileMenu->addAction("&Open...");
  openAction->setShortcut(QKeySequence::Open);

  connect(openAction, &QAction::triggered, this, &MainWindow::openImage);
}

void MainWindow::setupUI() {
  // create main MDI area
  mdiArea = new QMdiArea(this);
  mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setCentralWidget(mdiArea);

  // create a dock widget
  dock = new QDockWidget(this);
  histogramWidget = new HistogramWidget(dock);
  dock->setWidget(histogramWidget);

  addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::openImage() {
  QString filePath = QFileDialog::getOpenFileName(
      this, tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));

  QPixmap *pixmap = new QPixmap(filePath);
  if (pixmap->isNull()) {
    // TODO: display error message box
    return;
  }

  // create a new window
  MdiChild *mdiChild = new MdiChild();
  connect(mdiChild, &MdiChild::pixmapUpdated, histogramWidget, &HistogramWidget::updateHistogram);
  mdiChild->updatePixmap(*pixmap);

  mdiArea->addSubWindow(mdiChild);
  mdiChild->show();
}
