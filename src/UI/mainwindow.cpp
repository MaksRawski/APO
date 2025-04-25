#include "mainwindow.hpp"
#include "histogramWidget.hpp"
#include "mdiChild.hpp"
#include "parametersDialog.hpp"
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QSplitter>
#include <QTimer>
#include <qaction.h>
#include <qboxlayout.h>
#include <qfileinfo.h>
#include <qkeysequence.h>

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
  blurMeanAction = imageBlurMenu->addAction("&Mean");
  blurMedianAction = imageBlurMenu->addAction("Me&dian");
  blurGaussianAction = imageBlurMenu->addAction("&Gaussian");

  QMenu *imageEdgeDetectMenu = imageMenu->addMenu("&Edge detect");
  edgeDetectSobelAction = imageEdgeDetectMenu->addAction("&Sobel");
  edgeDetectLaplacianAction = imageEdgeDetectMenu->addAction("&Laplacian");
  edgeDetectCannyAction = imageEdgeDetectMenu->addAction("&Canny");
  edgeDetectPrewittAction = imageEdgeDetectMenu->addAction("&Prewitt");

  QMenu *imageSharpenMenu = imageMenu->addMenu("&Sharpen");
  sharpenLaplacianAction = imageSharpenMenu->addAction("&Laplacian");

  QMenu *imageCombineMenu = imageMenu->addMenu("Co&mbine");
  combineAddAction = imageCombineMenu->addAction("&Add");
  combineSubAction = imageCombineMenu->addAction("&Sub");

  QMenu *aboutMenu = menuBar()->addMenu("Info");
  aboutAction = aboutMenu->addAction("About");

  // actions that operate on single window
  for (auto c : getConnections())
    c.action->setEnabled(false);

  // actions that work on multiple images
  for (auto c : getMainWindowActions()) {
    c.action->setEnabled(false);
    connect(c.action, &QAction::triggered, this, c.slot);
  }

  // always available actions
  connect(openAction, &QAction::triggered, this, &MainWindow::openImage);
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
      this, tr("Open Image"), "",
      tr("Image Files (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm)"));

  if (filePath.isEmpty())
    return;

  QString fileName = QFileInfo(filePath).fileName();
  createImageWindow(ImageWrapper::fromPath(filePath));
  activeChild->setImageName(fileName);
}

void MainWindow::mdiSubWindowActivated(QMdiSubWindow *window) {
  disconnectActions(*activeChild);
  // if last window has been closed
  if (window == nullptr) {
    histogramWidget->reset();
    activeChild = nullptr;
    return;
  }
  MdiChild *mdiChild = qobject_cast<MdiChild *>(window);

  if (mdiChild) {
    activeChild = mdiChild;
    connectActions(*activeChild);
  }
}

void MainWindow::disconnectActions(const MdiChild &child) {
  for (auto c : getConnections())
    disconnect(c.action, &QAction::triggered, &child, c.slot);

  for (auto c : getMainWindowActions())
    disconnect(c.action, &QAction::triggered, this, c.slot);

  disconnect(&child, &MdiChild::imageUpdated, this, &MainWindow::toggleOptions);
  disconnect(&child, &MdiChild::imageUpdated, histogramWidget, &HistogramWidget::updateHistogram);
  for (auto c : getConnections())
    c.action->setEnabled(false);

  for (auto c : getMainWindowActions())
    c.action->setEnabled(false);
  histogramWidget->reset();
}

// enables all the actions that operate on the image
void MainWindow::connectActions(const MdiChild &child) {
  for (auto c : getConnections())
    connect(c.action, &QAction::triggered, &child, c.slot);

  for (auto c : getMainWindowActions())
    connect(c.action, &QAction::triggered, this, c.slot);

  connect(&child, &MdiChild::imageUpdated, this, &MainWindow::toggleOptions);
  connect(&child, &MdiChild::imageUpdated, histogramWidget, &HistogramWidget::updateHistogram);

  for (auto c : getConnections())
    c.action->setEnabled(true);

  for (auto c : getMainWindowActions())
    c.action->setEnabled(true);

  child.emitImageUpdatedSignal();
}

void MainWindow::splitChannels() {
  if (activeChild == nullptr)
    return; // should never happen
  QString imageName = activeChild->getImageName();
  QFileInfo fileInfo(imageName);
  QString baseName = fileInfo.completeBaseName();
  QString suffix = fileInfo.completeSuffix();
  QString format = pixelFormatToString(activeChild->getImage().getFormat());

  std::vector<ImageWrapper> channels = activeChild->getImage().splitChannels();
  int c = 0;
  for (ImageWrapper image : channels) {

    // create new window
    createImageWindow(image);
    activeChild->setImageName(baseName + '_' + format[c] + '.' + suffix);
    activeChild->setImageScale(activeChild->getImageScale());
    ++c;
  }
}

void MainWindow::toggleOptions(const ImageWrapper &image) {
  PixelFormat format = image.getFormat();
  if (format == PixelFormat::Grayscale8) {
    splitChannelsAction->setEnabled(false);
  } else {
    splitChannelsAction->setEnabled(true);
  }
}

void MainWindow::duplicateImage() {
  if (activeChild == nullptr)
    return; // should never happen

  MdiChild *dupChild = new MdiChild(activeChild->getImage());

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
  return {
      {toRGBAction, &MdiChild::toRGB},
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
      {blurMeanAction, &MdiChild::blurMean},
      {blurMedianAction, &MdiChild::blurMedian},
      {blurGaussianAction, &MdiChild::blurGaussian},
      {edgeDetectSobelAction, &MdiChild::edgeDetectSobel},
      {edgeDetectLaplacianAction, &MdiChild::edgeDetectLaplacian},
      {edgeDetectCannyAction, &MdiChild::edgeDetectCanny},
      {sharpenLaplacianAction, &MdiChild::sharpenLaplacian},
      {edgeDetectPrewittAction, &MdiChild::edgeDetectPrewitt},
  };
}

std::vector<MainWindowActionConnection> MainWindow::getMainWindowActions() const {
  return {{duplicateAction, &MainWindow::duplicateImage},
          {splitChannelsAction, &MainWindow::splitChannels},
          {combineAddAction, &MainWindow::combineAdd},
          {combineSubAction, &MainWindow::combineSub}};
}

std::vector<MdiChild*> MainWindow::getMdiChildren() const {
  const QList<QMdiSubWindow *> subWindows = mdiArea->subWindowList();
  std::vector<MdiChild*> mdiChildren;
  mdiChildren.reserve(subWindows.size());

  for (QMdiSubWindow *window : subWindows) {
    MdiChild *mdiChild = qobject_cast<MdiChild *>(window);
    if (mdiChild == nullptr)
      continue;
    mdiChildren.push_back(mdiChild);
  }
  return mdiChildren;
}

// scale the mdi child so that it takes at max 70% of the main window
void MainWindow::limitWindowSize(MdiChild &child) const {
  QSize size = child.getImageSize();
  QSize windowSize = this->size();
  QSize scaledSize = size.scaled(windowSize * 0.7, Qt::KeepAspectRatio);
  double scaleFactor = static_cast<float>(scaledSize.width()) / static_cast<float>(size.width());

  // prevent upscaling
  if (scaleFactor < 1.0) {
    child.setImageScale(scaleFactor);
    child.resize(scaledSize + CHILD_IMAGE_MARGIN);
    child.setMaximumSize(scaledSize + CHILD_IMAGE_MARGIN);
  }
}

void MainWindow::createImageWindow(const ImageWrapper &image) {
  disconnectActions(*activeChild);
  activeChild = new MdiChild(image);
  connectActions(*activeChild);

  limitWindowSize(*activeChild);
  mdiArea->addSubWindow(activeChild);
  activeChild->show();
}

void MainWindow::combineAdd() {
  combine([](cv::Mat f, cv::Mat s) -> cv::Mat { return f + s; });
  activeChild->setImageName("Result of Add");
}
void MainWindow::combineSub() {
  combine([](cv::Mat f, cv::Mat s) -> cv::Mat { return f - s; });
  activeChild->setImageName("Result of Sub");
}
namespace {
bool checkDimensions(QWidget *parent, const cv::Mat &first, const cv::Mat &second) {
  if (first.channels() != second.channels()) {
    QMessageBox::critical(parent, "Error", "Images must have the same number of channels!");
    return false;
  }
  if (first.rows != second.rows || first.cols != second.cols) {
    QMessageBox::critical(parent, "Error", "Images must have the same sizes!");
    return false;
  }
  return true;
}
std::vector<QString> getWindowNames(const std::vector<MdiChild *> &windows) {
  std::vector<QString> names;
  names.reserve(windows.size());

  for (uchar i = 0; i < windows.size(); ++i) {
    if (windows[i] == nullptr) {
      qDebug() << "window" << i << "is nullptr";
      continue;
    }
    QString name = windows[i]->getImageName();
    names.push_back(name);
  }
  return names;
}
int getActiveWindowIndex(MdiChild *activeChild, const std::vector<QString> names) {
  int activeWindowIndex = -1;
  if (activeChild != nullptr) {
    QString activeWindowName = activeChild->getImageName();
    auto it = std::find(names.begin(), names.end(), activeWindowName);
    if (it != names.end()) {
      activeWindowIndex = it - names.begin();
    }
  }
  return activeWindowIndex;
}
} // namespace

void MainWindow::combine(cv::Mat (*op)(cv::Mat, cv::Mat)) {
  std::vector<MdiChild*> windows = getMdiChildren();
  std::vector<QString> names = getWindowNames(windows);
  int activeWindowIndex = getActiveWindowIndex(activeChild, names);

  auto res = windowsPairDialog(this, names, activeWindowIndex);
  if (!res.has_value())
    return;

  uchar firstI, secondI;
  std::tie(firstI, secondI) = res.value();
  cv::Mat first = windows[firstI]->getImage().getMat();
  cv::Mat second = windows[secondI]->getImage().getMat();
  if (!checkDimensions(this, first, second)) return;

  cv::Mat out = op(first, second);
  createImageWindow(out);
}
