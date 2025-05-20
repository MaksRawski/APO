#include "mainwindow.hpp"
#include "histogramWidget.hpp"
#include "mdiChild.hpp"
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

  /// PRE PROCESSING
  QMenu *preMenu = menuBar()->addMenu("P&reprocessing");

  QMenu *contrastMenu = preMenu->addMenu("&Contrast");
  negateAction = contrastMenu->addAction("&Negate");
  negateAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
  normalizeAction = contrastMenu->addAction("Nor&malize");
  equalizeAction = contrastMenu->addAction("&Equalize");
  rangeStretchAction = contrastMenu->addAction("Range &stretch");
  posterizeAction = contrastMenu->addAction("&Posterize");

  QMenu *blurMenu = preMenu->addMenu("&Blur");
  blurMeanAction = blurMenu->addAction("&Mean");
  blurMedianAction = blurMenu->addAction("Me&dian");
  blurGaussianAction = blurMenu->addAction("&Gaussian");

  QMenu *sharpenMenu = preMenu->addMenu("&Sharpen");
  sharpenLaplacianAction = sharpenMenu->addAction("&Laplacian");

  QMenu *customMenu = preMenu->addMenu("C&ustom");
  customMaskAction = customMenu->addAction("&Mask");
  custom2StageAction = customMenu->addAction("2-&Stage filtering");

  QMenu *morphMenu = preMenu->addMenu("&Morphology");
  morphologyErosionAction = morphMenu->addAction("&Erosion");
  morphologyDilationAction = morphMenu->addAction("&Dilation");
  morphologyOpenAction = morphMenu->addAction("&Open");
  morphologyCloseAction = morphMenu->addAction("&Close");
  morphologySkeletonizeAction = morphMenu->addAction("&Skeletonize");

  /// SEGMENTATION
  QMenu *segmentationMenu = menuBar()->addMenu("&Segmentation");

  QMenu *thresholdingMenu = segmentationMenu->addMenu("&Thresholding");
  thresholdManualAction = thresholdingMenu->addAction("&Manual");
  thresholdAdaptiveAction = thresholdingMenu->addAction("&Adaptive");
  thresholdOtsuAction = thresholdingMenu->addAction("&Otsu");

  QMenu *edgeDetectMenu = segmentationMenu->addMenu("&Edge detection");
  edgeDetectSobelAction = edgeDetectMenu->addAction("&Sobel");
  edgeDetectLaplacianAction = edgeDetectMenu->addAction("&Laplacian");
  edgeDetectCannyAction = edgeDetectMenu->addAction("&Canny");
  edgeDetectPrewittAction = edgeDetectMenu->addAction("&Prewitt");

  /// POST PROCESSING
  QMenu *postMenu = menuBar()->addMenu("P&ostprocessing");
  QMenu *combineMenu = postMenu->addMenu("&Combine");
  combineAddAction = combineMenu->addAction("&Add");
  combineSubAction = combineMenu->addAction("&Sub");
  combineBlendAction = combineMenu->addAction("&Blend");
  combineANDAction = combineMenu->addAction("A&ND");
  combineORAction = combineMenu->addAction("&OR");
  combineXORAction = combineMenu->addAction("&XOR");

  /// ANALYSIS
  QMenu *analysisMenu = menuBar()->addMenu("&Analysis");
  houghAction = analysisMenu->addAction("&Hough transform");
  profileLineAction = analysisMenu->addAction("&Line intensity profile");
  profileLineAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_K));

  QMenu *aboutMenu = menuBar()->addMenu("Info");
  aboutAction = aboutMenu->addAction("About");

  // actions that operate on a single window
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
  createImageWindow(ImageWrapper::fromPath(filePath), fileName);
}

void MainWindow::mdiSubWindowActivated(QMdiSubWindow *window) {
  if (activeChild)
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
  auto format = PixelFormatUtils::toString(activeChild->getImage().getFormat());

  std::vector<ImageWrapper> channels = activeChild->getImage().splitChannels();
  int c = 0;
  for (ImageWrapper image : channels) {
    createImageWindow(image, baseName + '_' + format[c] + '.' + suffix);
    ++c;
  }
}

void MainWindow::toggleOptions(const ImageWrapper &image) {
  PixelFormat format = image.getFormat();
  if (format == PixelFormat::Grayscale8) {
    splitChannelsAction->setEnabled(false);
    houghAction->setEnabled(true);
    profileLineAction->setEnabled(true);
  } else {
    splitChannelsAction->setEnabled(true);
    houghAction->setEnabled(false);
    profileLineAction->setEnabled(false);
  }
}

void MainWindow::duplicateImage() {
  if (activeChild == nullptr)
    return; // should never happen

  MdiChild *dupChild = new MdiChild(activeChild->getImage());

  auto dupName = activeChild->getImageBasename() + "_dup." + activeChild->getImageNameSuffix();
  dupChild->setImageName(dupName);
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
      {customMaskAction, &MdiChild::customMask},
      {custom2StageAction, &MdiChild::customTwoStageFilter},
      {morphologyErosionAction, &MdiChild::morphologyErode},
      {morphologyDilationAction, &MdiChild::morphologyDilate},
      {morphologyOpenAction, &MdiChild::morphologyOpen},
      {morphologyCloseAction, &MdiChild::morphologyClose},
      {morphologySkeletonizeAction, &MdiChild::morphologySkeletonize},
      {houghAction, &MdiChild::houghTransform},
      {thresholdManualAction, &MdiChild::thresholdManual},
      {thresholdAdaptiveAction, &MdiChild::thresholdAdaptive},
      {thresholdOtsuAction, &MdiChild::thresholdOtsu},
      {profileLineAction, &MdiChild::profileLine},
  };
}

std::vector<MainWindowActionConnection> MainWindow::getMainWindowActions() const {
  return {{duplicateAction, &MainWindow::duplicateImage},
          {splitChannelsAction, &MainWindow::splitChannels},
          {combineAddAction, &MainWindow::combineAdd},
          {combineSubAction, &MainWindow::combineSub},
          {combineBlendAction, &MainWindow::combineBlend},
          {combineANDAction, &MainWindow::combineAND},
          {combineORAction, &MainWindow::combineOR},
          {combineXORAction, &MainWindow::combineXOR}};
}

std::vector<MdiChild *> MainWindow::getMdiChildren() const {
  const QList<QMdiSubWindow *> subWindows = mdiArea->subWindowList();
  std::vector<MdiChild *> mdiChildren;
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
  double scaleFactor = static_cast<double>(scaledSize.width()) / static_cast<double>(size.width());

  // prevent upscaling
  if (scaleFactor < 1.0) {
    child.resize(scaledSize + CHILD_IMAGE_MARGIN);
    child.fitImage();
  }
}

void MainWindow::createImageWindow(const ImageWrapper &image, const QString &name = nullptr) {
  if (activeChild != nullptr)
    disconnectActions(*activeChild);
  activeChild = new MdiChild(image);
  connectActions(*activeChild);

  limitWindowSize(*activeChild);
  mdiArea->addSubWindow(activeChild);
  activeChild->show();
  if (name != nullptr)
    activeChild->setImageName(name);
}

void MainWindow::combineAdd() {
  combine([](cv::Mat f, cv::Mat s) -> cv::Mat { return f + s; }, "Result of Add");
}
void MainWindow::combineSub() {
  combine([](cv::Mat f, cv::Mat s) -> cv::Mat { return f - s; }, "Result of Sub");
}
void MainWindow::combineAND() {
  combine([](cv::Mat f, cv::Mat s) -> cv::Mat { return f & s; }, "Result of AND");
}
void MainWindow::combineOR() {
  combine([](cv::Mat f, cv::Mat s) -> cv::Mat { return f | s; }, "Result of OR");
}
void MainWindow::combineXOR() {
  combine([](cv::Mat f, cv::Mat s) -> cv::Mat { return f ^ s; }, "Result of XOR");
}

namespace {
bool haveSameDimensions(QWidget *parent, const cv::Mat &first, const cv::Mat &second) {
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
      activeWindowIndex = static_cast<int>(it - names.begin());
    }
  }
  return activeWindowIndex;
}
} // namespace

void MainWindow::combineBlend() {
  std::vector<MdiChild *> windows = getMdiChildren();
  std::vector<QString> names = getWindowNames(windows);
  uint activeWindowIndex = getActiveWindowIndex(activeChild, names);
  uint secondWindowIndex = (activeWindowIndex + 1) % names.size();

  auto id = [](auto i) { return i; };
  auto mat =
      Dialog(this, QString("Select two windows"),                                               //
             InputSpec<EnumVariantParam<uint>>{"First window", {names}, activeWindowIndex, id}, //
             InputSpec<EnumVariantParam<uint>>{"Second window", {names}, secondWindowIndex, id},
             InputSpec<IntParam>{"Blend percentage", {0, 100}, 50})
          .runWithPreview(
              [this, windows](uint i1, uint i2, int blendValue) -> std::optional<cv::Mat> {
                cv::Mat first = windows[i1]->getImage().getMat();
                cv::Mat second = windows[i2]->getImage().getMat();
                if (!haveSameDimensions(this, first, second))
                  return std::nullopt;

                double blendFactor = blendValue / 100.0;
                return first * blendFactor + second * (1 - blendFactor);
              });

  if (mat.has_value())
    createImageWindow(mat.value(), "Result of Blend");
}

void MainWindow::combine(std::function<cv::Mat(cv::Mat, cv::Mat)> op,
                         const QString &name = nullptr) {
  std::vector<MdiChild *> windows = getMdiChildren();
  std::vector<QString> names = getWindowNames(windows);
  uint activeWindowIndex = getActiveWindowIndex(activeChild, names);
  uint secondWindowIndex = (activeWindowIndex + 1) % names.size();

  auto id = [](auto i) { return i; };
  auto mat =
      Dialog(this, QString("Select two windows"),                                               //
             InputSpec<EnumVariantParam<uint>>{"First window", {names}, activeWindowIndex, id}, //
             InputSpec<EnumVariantParam<uint>>{"Second window", {names}, secondWindowIndex, id})
          .runWithPreview([this, windows, op](uint i1, uint i2) -> std::optional<cv::Mat> {
            cv::Mat first = windows[i1]->getImage().getMat();
            cv::Mat second = windows[i2]->getImage().getMat();
            if (!haveSameDimensions(this, first, second))
              return std::nullopt;

            return op(first, second);
          });

  if (mat.has_value())
    createImageWindow(mat.value(), name);
}
