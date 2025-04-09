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
#include <QSplitter>
#include <qaction.h>
#include <qmdiarea.h>
#include <qmdisubwindow.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qpixmap.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setupMenuBar();
  setupUI();
}

void MainWindow::setupMenuBar() {
  QMenu *fileMenu = menuBar()->addMenu("&File");
  QAction *openAction = fileMenu->addAction("&Open...");
  openAction->setShortcut(QKeySequence::Open);

  QAction *saveAction = fileMenu->addAction("&Save");
  saveAction->setEnabled(false);
  saveAction->setShortcut(QKeySequence::Save);

  QMenu *imageMenu = menuBar()->addMenu("&Image");
  QMenu *imageTypeMenu = imageMenu->addMenu("&Type");

  toLabAction = imageTypeMenu->addAction("&Lab");
  toLabAction->setEnabled(false);
  toHSVAction = imageTypeMenu->addAction("&HSV");
  toHSVAction->setEnabled(false);
  toRGBAction = imageTypeMenu->addAction("&RGB");
  toRGBAction->setEnabled(false);
  toGrayscaleAction = imageTypeMenu->addAction("&Grayscale");
  toGrayscaleAction->setEnabled(false);

  connect(openAction, &QAction::triggered, this, &MainWindow::openImage);
  // connect(saveAction, &QAction::triggered, this, &MainWindow::saveImage);
}

void MainWindow::setupUI() {
  // create main MDI area
  mdiArea = new QMdiArea(this);
  mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  connect(mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::mdiSubWindowActivated);
  setCentralWidget(mdiArea);

  // create dock
  QDockWidget *dock = new QDockWidget;

  // for now using just HistogramWidget as the entire dock
  histogramWidget = new HistogramWidget;
  dock->setWidget(histogramWidget);

  addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::openImage() {
  QString filePath = QFileDialog::getOpenFileName(
      this, tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));
  if (filePath.isEmpty()) return;

  // create new window
  MdiChild *newChild = new MdiChild;

  // NOTE: mdiChild(ren) will emit signals anytime an image changes so technically we could have a non-active
  // image trigger this and overwrite the histogram output. For now I assume that for image to change
  // it must be first active, so the histogram should always match the active image.
  newChild->loadImage(filePath); // will emit the initial imageUpdated signal

  connectActions(newChild);

  // scale the image so that it takes at max 70% of the window
  QSize size = activeChild->getImageSize();
  QSize windowSize = this->size();
  QSize scaledSize = size.scaled(windowSize * 0.7, Qt::KeepAspectRatio);
  double scaleFactor = static_cast<float>(scaledSize.width()) / static_cast<float>(size.width());

  // prevent upscaling
  if (scaleFactor < 1.0)
    activeChild->setImageScale(scaleFactor);

  mdiArea->addSubWindow(activeChild);
  activeChild->show();
}

void MainWindow::mdiSubWindowActivated(QMdiSubWindow *window) {
  if (window == nullptr) {
    histogramWidget->reset();
    return;
  }
  MdiChild *mdiChild = qobject_cast<MdiChild*>(window);

  if (mdiChild) {
    activeChild = mdiChild;
    histogramWidget->updateHistogram(mdiChild->getImage());
  }
}


// enables all the actions that operate on the image
void MainWindow::connectActions(MdiChild *newActiveChild) {
  disconnect(toLabAction, &QAction::triggered, activeChild, &MdiChild::toLab);
  disconnect(toHSVAction, &QAction::triggered, activeChild, &MdiChild::toHSV);
  disconnect(toRGBAction, &QAction::triggered, activeChild, &MdiChild::toRGB);
  disconnect(toGrayscaleAction, &QAction::triggered, activeChild, &MdiChild::toGrayscale);
  disconnect(activeChild, &MdiChild::imageUpdated, histogramWidget, &HistogramWidget::updateHistogram);

  activeChild = newActiveChild;
  connect(toLabAction, &QAction::triggered, activeChild, &MdiChild::toLab);
  connect(toHSVAction, &QAction::triggered, activeChild, &MdiChild::toHSV);
  connect(toRGBAction, &QAction::triggered, activeChild, &MdiChild::toRGB);
  connect(toGrayscaleAction, &QAction::triggered, activeChild, &MdiChild::toGrayscale);
  connect(activeChild, &MdiChild::imageUpdated, histogramWidget, &HistogramWidget::updateHistogram);

  toGrayscaleAction->setEnabled(true);
  toRGBAction->setEnabled(true);
  toLabAction->setEnabled(true);
  toGrayscaleAction->setEnabled(true);
}
