#include "mainwindow.hpp"
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

void MainWindow::openImage() {
  QString filePath = QFileDialog::getOpenFileName(
      this, tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));

  QPixmap *pixmap = new QPixmap(filePath);
  if (pixmap->isNull()) {
    // TODO: display error message box
    return;
  }
  MdiChild *win = new MdiChild(*pixmap);
  mdiArea->addSubWindow(win);
  win->show();
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
  histogramDock = new QDockWidget(tr("Histogram"), this);
  auto *tmpDockContent = new QTextEdit();
  histogramDock->setWidget(tmpDockContent);

  addDockWidget(Qt::RightDockWidgetArea, histogramDock);
}
