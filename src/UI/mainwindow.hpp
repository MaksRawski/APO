#pragma once

#include "histogramWidget.hpp"
#include "mdiChild.hpp"
#include <QLabel>
#include <QMainWindow>
#include <QMdiArea>
#include <QScrollArea>
#include <qdockwidget.h>
#include <qimage.h>
#include <qmdisubwindow.h>
#include <qpixmap.h>
#include <qtabwidget.h>

namespace {
// associates QAction displayed in the menu with appropriate function in MdiChild
struct ActionConnection {
  QAction *action;
  void (MdiChild::*slot)();
};
// associates QAction displayed in the menu with appropriate function in MainWindow
// NOTE: since MainWindow is declared later here's just a declaration, its definition is below
// MainWindow definition
struct MainWindowActionConnection;
} // namespace

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);

private:
  QMdiArea *mdiArea;
  HistogramWidget *histogramWidget;
  MdiChild *activeChild = nullptr;

  // actions
  QAction *toRGBAction;
  QAction *toHSVAction;
  QAction *toLabAction;
  QAction *toGrayscaleAction;
  QAction *splitChannelsAction;
  QAction *duplicateAction;
  QAction *aboutAction;
  QAction *negateAction;
  QAction *normalizeAction;
  QAction *equalizeAction;
  QAction *rangeStretchAction;
  QAction *saveAction;
  QAction *renameAction;
  QAction *posterizeAction;
  QAction *blurMeanAction;
  QAction *blurMedianAction;
  QAction *blurGaussianAction;
  QAction *edgeDetectSobelAction;
  QAction *edgeDetectLaplacianAction;
  QAction *edgeDetectCannyAction;
  QAction *sharpenLaplacianAction;
  QAction *edgeDetectPrewittAction;
  QAction *combineAddAction;
  QAction *combineSubAction;
  QAction *combineBlendAction;
  QAction *combineANDAction;
  QAction *combineORAction;
  QAction *combineXORAction;
  QAction *customMaskAction;
  QAction *custom2StageAction;
  QAction *morphologyErosionAction;
  QAction *morphologyDilationAction;
  QAction *morphologyOpenAction;
  QAction *morphologyCloseAction;
  QAction *morphologySkeletonizeAction;
  QAction *houghAction;

  void connectActions(const MdiChild &child);
  void disconnectActions(const MdiChild &child);
  std::vector<ActionConnection> getConnections() const;
  std::vector<MainWindowActionConnection> getMainWindowActions() const;

  // setup functions
  void setupMenuBar();
  void setupUI();

  // utils
  std::vector<MdiChild *> getMdiChildren() const;
  void limitWindowSize(MdiChild &child) const;
  void combine(std::function<cv::Mat(cv::Mat, cv::Mat)> op, const QString &name);
  void createImageWindow(const ImageWrapper &image, const QString &name);

private slots:
  void openAboutWindow();
  void openImage();
  void duplicateImage();
  void splitChannels();
  void toggleOptions(const ImageWrapper &image);
  void mdiSubWindowActivated(QMdiSubWindow *window);
  void combineAdd();
  void combineSub();
  void combineBlend();
  void combineAND();
  void combineOR();
  void combineXOR();
};

namespace {
struct MainWindowActionConnection {
  QAction *action;
  void (MainWindow::*slot)();
};
} // namespace
