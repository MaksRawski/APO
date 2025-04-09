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
#include <cassert>
#include <qaction.h>
#include <qfileinfo.h>
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
  splitChannelsAction = imageMenu->addAction("&Split channels");

  QMenu *imageTypeMenu = imageMenu->addMenu("&Type");
  toRGBAction = imageTypeMenu->addAction("&RGB");
  toHSVAction = imageTypeMenu->addAction("&HSV");
  toLabAction = imageTypeMenu->addAction("&Lab");
  toGrayscaleAction = imageTypeMenu->addAction("&Grayscale");

  splitChannelsAction->setEnabled(false);
  toRGBAction->setEnabled(false);
  toHSVAction->setEnabled(false);
  toLabAction->setEnabled(false);
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
  if (filePath.isEmpty())
    return;

  // create new window and set it as the active one
  activeChild = new MdiChild;

  // NOTE: will emit the initial imageUpdated signal
  activeChild->loadImage(filePath);
  connectActions(activeChild);

  // scale the image so that it takes at max 70% of the window
  QSize size = activeChild->getImageSize();
  QSize windowSize = this->size();
  QSize scaledSize = size.scaled(windowSize * 0.7, Qt::KeepAspectRatio);
  double scaleFactor =
      static_cast<float>(scaledSize.width()) / static_cast<float>(size.width());

  // prevent upscaling
  if (scaleFactor < 1.0) {
    activeChild->setImageScale(scaleFactor);
    qDebug() << "initial scale:" << activeChild->getImageScale();
    activeChild->resize(scaledSize + CHILD_IMAGE_MARGIN);
  }

  mdiArea->addSubWindow(activeChild);
  activeChild->show();
}

void MainWindow::mdiSubWindowActivated(QMdiSubWindow *window) {
  if (window == nullptr) {
    histogramWidget->reset();
    activeChild = nullptr;
    return;
  }
  MdiChild *mdiChild = qobject_cast<MdiChild *>(window);

  if (mdiChild) {
    activeChild = mdiChild;
    histogramWidget->updateHistogram(mdiChild->getImage());
  }
}

// enables all the actions that operate on the image
void MainWindow::connectActions(MdiChild *newActiveChild) {
  if (activeChild != nullptr) {
    disconnect(splitChannelsAction, &QAction::triggered, this, &MainWindow::splitChannels);
    disconnect(toRGBAction, &QAction::triggered, activeChild, &MdiChild::toRGB);
    disconnect(toHSVAction, &QAction::triggered, activeChild, &MdiChild::toHSV);
    disconnect(toLabAction, &QAction::triggered, activeChild, &MdiChild::toLab);
    disconnect(toGrayscaleAction, &QAction::triggered, activeChild, &MdiChild::toGrayscale);

    disconnect(activeChild, &MdiChild::imageUpdated, histogramWidget, &HistogramWidget::updateHistogram);
    disconnect(activeChild, &MdiChild::imageUpdated, this, &MainWindow::toggleOptions);
  }

  activeChild = newActiveChild;
  connect(splitChannelsAction, &QAction::triggered, this, &MainWindow::splitChannels);
  connect(toLabAction, &QAction::triggered, activeChild, &MdiChild::toLab);
  connect(toHSVAction, &QAction::triggered, activeChild, &MdiChild::toHSV);
  connect(toRGBAction, &QAction::triggered, activeChild, &MdiChild::toRGB);
  connect(toGrayscaleAction, &QAction::triggered, activeChild, &MdiChild::toGrayscale);

  connect(activeChild, &MdiChild::imageUpdated, histogramWidget, &HistogramWidget::updateHistogram);
  connect(activeChild, &MdiChild::imageUpdated, this, &MainWindow::toggleOptions);

  splitChannelsAction->setEnabled(true);
  toGrayscaleAction->setEnabled(true);
  toRGBAction->setEnabled(true);
  toLabAction->setEnabled(true);
  toHSVAction->setEnabled(true);
}

void MainWindow::splitChannels() {
  if (activeChild == nullptr) return; // should never happen
  QString imageName = activeChild->getImageName();
  QFileInfo fileInfo(imageName);
  QString baseName = fileInfo.completeBaseName();
  QString suffix = fileInfo.completeSuffix();
  QString format = pixelFormatToString(activeChild->getImage().getFormat());

  std::vector<ImageWrapper> channels = activeChild->getImage().splitChannels();
  int c = 0;
  for (ImageWrapper image : channels) {
    // create new window
    MdiChild *newChild = new MdiChild;

    // NOTE: will emit the initial imageUpdated signal
    newChild->setImage(image);
    newChild->setImageName(baseName + '_' + format[c] + '.' + suffix);

    newChild->setImageScale(activeChild->getImageScale());
    newChild->resize(activeChild->size());
    connectActions(newChild);

    mdiArea->addSubWindow(newChild);
    newChild->show();
    ++c;
  }
}

void MainWindow::toggleOptions(const ImageWrapper &image){
  PixelFormat format = image.getFormat();
  if (format == PixelFormat::Grayscale8) {
    splitChannelsAction->setEnabled(false);
  } else {
    splitChannelsAction->setEnabled(true);
  }
}
