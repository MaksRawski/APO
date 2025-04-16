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

const int INITIAL_WIDTH = 400;
const int INITIAL_HEIGHT = 400;

namespace {
struct ActionConnection {
  QAction *action;
  void (MdiChild::*slot)();
};
struct MainWindowActionConnection;
struct NameWindow {
  QString name;
  MdiChild *window;
};
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
  void connectActions(const MdiChild *child);
  void disconnectActions(const MdiChild *child);
  std::vector<ActionConnection> getConnections() const;
  std::vector<MainWindowActionConnection> getMainWindowActions() const;

  // setup functions
  void setupMenuBar();
  void setupUI();

  // utils
  std::vector<NameWindow> getWindows() const;

private slots:
  void openAboutWindow();
  void openImage();
  void duplicateImage();
  void splitChannels();
  void toggleOptions(const ImageWrapper &image);
  void mdiSubWindowActivated(QMdiSubWindow *window);
  void combineAdd();
  void combineSub();
};

namespace {
struct MainWindowActionConnection {
  QAction *action;
  void (MainWindow::*slot)();
};
} // namespace
