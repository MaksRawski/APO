#include "mainwindow.h"
#include <QAction>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QPixmap>
#include <QScrollArea>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  QMenu *fileMenu = menuBar()->addMenu("&File");
  QAction *openAction = fileMenu->addAction("&Open...");

  connect(openAction, &QAction::triggered, this, &MainWindow::openImage);

  // Initial setup for displaying the image
  scrollArea = new QScrollArea(this);
  imageLabel = new QLabel(scrollArea);

  imageLabel->setAlignment(Qt::AlignCenter);
  scrollArea->setWidget(imageLabel);

  setCentralWidget(scrollArea);
}

void MainWindow::openImage() {
  QString fileName = QFileDialog::getOpenFileName(
      this, tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));

  if (!fileName.isEmpty()) {
    QPixmap pixmap(fileName);
    if (!pixmap.isNull()) {
      imageLabel->setPixmap(pixmap);
      scrollArea->setVisible(true);
      imageLabel->adjustSize();
    } else {
      // Handle image load error (e.g., show a message box)
      qDebug() << "Failed to load image: " << fileName;
    }
  }
}
