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

  connect(openAction, &QAction::triggered, this, &MainWindow::openImage);
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
  QString fileName = QFileInfo(filePath).fileName();

  QPixmap *pixmap = new QPixmap(filePath);

  if (pixmap->isNull()) {
    // TODO: display error message box
    return;
  }

  // create new window
  MdiChild *mdiChild = new MdiChild;

  // NOTE: mdiChild(ren) will emit signals anytime an image changes so technically we could have a non-active
  // image trigger this and overwrite the histogram output. For now I assume that for image to change
  // it must be first active, so the histogram should always match the active image.
  connect(mdiChild, &MdiChild::pixmapUpdated, histogramWidget, &HistogramWidget::updateHistogram);
  mdiChild->updatePixmap(*pixmap, fileName); // set the pixmap. will also emit the appropriate signal

  // scale the image so that it takes at max 70% of the window
  QSize size = pixmap->size();
  QSize windowSize = this->size();
  QSize scaledSize = size.scaled(windowSize * 0.7, Qt::KeepAspectRatio);
  double scaleFactor = static_cast<float>(scaledSize.width()) / static_cast<float>(size.width());
  // prevent upscaling
  if (scaleFactor < 1.0)
    mdiChild->setImageScale(scaleFactor);

  mdiArea->addSubWindow(mdiChild);
  mdiChild->show();
}

void MainWindow::mdiSubWindowActivated(QMdiSubWindow *window) {
  if (window == nullptr) {
    histogramWidget->reset();
    return;
  }
  MdiChild *mdiChild = qobject_cast<MdiChild*>(window);

  if (mdiChild) {
    histogramWidget->updateHistogram(mdiChild->getPixmap());
  }
}
