#include "mainwindow.hpp"
#include "histogramWidget.hpp"
#include "mdiChild.hpp"
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QPixmap>
#include <QSplitter>
#include <qaction.h>
#include <qboxlayout.h>
#include <qfileinfo.h>
#include <qkeysequence.h>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setupMenuBar();
  setupUI();
}

void MainWindow::setupMenuBar() {
  QMenu *fileMenu = menuBar()->addMenu("&File");
  QAction *openAction = fileMenu->addAction("&Open");
  openAction->setShortcut(QKeySequence::Open);

  saveAction = fileMenu->addAction("&Save");
  saveAction->setEnabled(false);
  saveAction->setShortcut(QKeySequence::Save);

  QMenu *imageMenu = menuBar()->addMenu("&Image");
  duplicateAction = imageMenu->addAction("&Duplicate");
  duplicateAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
  renameAction = imageMenu->addAction("&Rename");
  renameAction->setShortcut(QKeySequence(Qt::Key_F2));

  QMenu *imageTypeMenu = imageMenu->addMenu("&Type");
  toRGBAction = imageTypeMenu->addAction("&RGB");
  toHSVAction = imageTypeMenu->addAction("&HSV");
  toLabAction = imageTypeMenu->addAction("&Lab");
  toGrayscaleAction = imageTypeMenu->addAction("&Grayscale");
  toGrayscaleAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
  splitChannelsAction = imageTypeMenu->addAction("Split &channels");
  splitChannelsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_C));

  QMenu *imageContrastMenu = imageMenu->addMenu("&Contrast");
  negateAction = imageContrastMenu->addAction("&Negate");
  negateAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
  normalizeAction = imageContrastMenu->addAction("Nor&malize");
  equalizeAction = imageContrastMenu->addAction("&Equalize");
  rangeStretchAction = imageContrastMenu->addAction("Range &stretch");
  posterizeAction = imageContrastMenu->addAction("&Posterize");

  QMenu *imageBlurMenu = imageMenu->addMenu("&Blur");
  blurMedianAction = imageBlurMenu->addAction("Median");
  blurGaussianAction = imageBlurMenu->addAction("Gaussian");


  QMenu *aboutMenu = menuBar()->addMenu("Info");
  aboutAction = aboutMenu->addAction("About");

  for (auto c : getConnections())
    c.action->setEnabled(false);

  // NOTE: connections that need MdiChild directly have to be created using `connectActions`
  connect(openAction, &QAction::triggered, this, &MainWindow::openImage);
  connect(duplicateAction, &QAction::triggered, this, &MainWindow::duplicateImage);
  connect(splitChannelsAction, &QAction::triggered, this, &MainWindow::splitChannels);
  connect(aboutAction, &QAction::triggered, this, &MainWindow::openAboutWindow);
}

void MainWindow::setupUI() {
  // create main MDI area
  mdiArea = new QMdiArea(this);
  mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  connect(mdiArea, &QMdiArea::subWindowActivated, this, &MainWindow::mdiSubWindowActivated);
  setCentralWidget(mdiArea);

  // create dock
  QDockWidget *dock = new QDockWidget(this);

  // for now using just HistogramWidget as the entire dock
  histogramWidget = new HistogramWidget;
  dock->setWidget(histogramWidget);
  addDockWidget(Qt::RightDockWidgetArea, dock);

  // wait with resizing till the UI fully renders
  QTimer::singleShot(0, [this, dock]() {
    if (dock->isVisible()) {
      int w = static_cast<int>(0.25 * this->width());
      dock->setMinimumWidth(w);
    }
  });
}

void MainWindow::openImage() {
  QString filePath = QFileDialog::getOpenFileName(
      this, tr("Open Image"), "", tr("Image Files (*.png *.jpg *.bmp)"));
  if (filePath.isEmpty())
    return;

  // create new window and connect actions to it
  disconnectActions(activeChild);
  activeChild = new MdiChild;
  connectActions(activeChild);

  // NOTE: will emit the initial imageUpdated signal
  activeChild->loadImage(filePath);

  // scale the image so that it takes at max 70% of the window
  QSize size = activeChild->getImageSize();
  QSize windowSize = this->size();
  QSize scaledSize = size.scaled(windowSize * 0.7, Qt::KeepAspectRatio);
  double scaleFactor = static_cast<float>(scaledSize.width()) / static_cast<float>(size.width());

  // prevent upscaling
  if (scaleFactor < 1.0) {
    activeChild->setImageScale(scaleFactor);
    activeChild->resize(scaledSize + CHILD_IMAGE_MARGIN);
    activeChild->setMaximumSize(scaledSize + CHILD_IMAGE_MARGIN);
  }

  mdiArea->addSubWindow(activeChild);
  activeChild->show();
}

void MainWindow::mdiSubWindowActivated(QMdiSubWindow *window) {
  // if last window has been closed
  disconnectActions(activeChild);
  if (window == nullptr) {
    histogramWidget->reset();
    activeChild = nullptr;
    return;
  }
  MdiChild *mdiChild = qobject_cast<MdiChild *>(window);

  if (mdiChild) {
    activeChild = mdiChild;
    histogramWidget->updateHistogram(mdiChild->getImage());
    connectActions(activeChild);
  }
}

void MainWindow::disconnectActions(const MdiChild *child) {
  if (child != nullptr) {
    for (auto c : getConnections()) {
      disconnect(c.action, &QAction::triggered, child, c.slot);
    }
    disconnect(child, &MdiChild::imageUpdated, this, &MainWindow::toggleOptions);
    disconnect(child, &MdiChild::imageUpdated, histogramWidget, &HistogramWidget::updateHistogram);
  }
  for (auto c : getConnections()) {
    c.action->setEnabled(false);
  }
}

// enables all the actions that operate on the image
void MainWindow::connectActions(const MdiChild *child) {
  if (child == nullptr)
    throw new std::runtime_error("Tried to make connections to nullptr!");

  for (auto c : getConnections()) {
    connect(c.action, &QAction::triggered, child, c.slot);
  }
  connect(child, &MdiChild::imageUpdated, this, &MainWindow::toggleOptions);
  connect(child, &MdiChild::imageUpdated, histogramWidget, &HistogramWidget::updateHistogram);

  for (auto c : getConnections()) {
    c.action->setEnabled(true);
  }
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
    disconnectActions(activeChild);
    connectActions(newChild);

    // NOTE: will emit the initial imageUpdated signal
    newChild->setImage(image);
    newChild->setImageName(baseName + '_' + format[c] + '.' + suffix);
    newChild->setImageScale(activeChild->getImageScale());
    newChild->resize(activeChild->size());

    activeChild = newChild;
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

void MainWindow::duplicateImage() {
  if (activeChild == nullptr) return; // should never happen

  MdiChild *dupChild = new MdiChild;
  dupChild->setImage(activeChild->getImage());

  auto dupName = activeChild->getImageBasename() + "_dup." + activeChild->getImageNameSuffix();
  dupChild->setImageName(dupName);

  dupChild->setImageScale(activeChild->getImageScale());
  dupChild->resize(activeChild->size());

  mdiArea->addSubWindow(dupChild);
  dupChild->show();
}

void MainWindow::openAboutWindow() {
  QDialog window;
  QVBoxLayout *layout = new QVBoxLayout(&window);

  window.resize(320, 240);
  window.setWindowTitle("O programie");
  QLabel *title = new QLabel("<h3>Aplikacja zbiorcza z ćwiczeń laboratoryjnych</h3>");
  QLabel *author = new QLabel("Autor: Maksymilian Rawski");
  QLabel *instructor = new QLabel("Prowadzący: dr inż. Łukasz Roszkowiak");
  QLabel *className = new QLabel("Algorytmy Przetwarzania Obrazów 2024");
  QLabel *group = new QLabel("WIT grupa ID06IO1");

  layout->addWidget(title);
  layout->addWidget(author);
  layout->addWidget(instructor);
  layout->addWidget(className);
  layout->addWidget(group);
  window.exec();
}

std::vector<ActionConnection> MainWindow::getConnections() const {
  return {{toRGBAction, &MdiChild::toRGB},
          {toHSVAction, &MdiChild::toHSV},
          {toLabAction, &MdiChild::toLab},
          {toGrayscaleAction, &MdiChild::toGrayscale},
          {negateAction, &MdiChild::negate},
          {normalizeAction, &MdiChild::normalize},
          {equalizeAction, &MdiChild::equalize},
          {rangeStretchAction, &MdiChild::rangeStretch},
          {saveAction, &MdiChild::save},
          {renameAction, &MdiChild::rename},
          {posterizeAction, &MdiChild::posterize},
          {blurMedianAction, &MdiChild::blurMedian},
          {blurGaussianAction, &MdiChild::blurGaussian}};
}
