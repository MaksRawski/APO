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

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);

private:
  QMdiArea *mdiArea;
  // necessary to store, to allow for communication
  HistogramWidget *histogramWidget;
  MdiChild *activeChild;

  // actions
  QAction *toRGBAction;
  QAction *toHSVAction;
  QAction *toLabAction;
  QAction *toGrayscaleAction;
  void connectActions(MdiChild *newActiveChild);

  // setup functions
  void setupMenuBar();
  void setupUI();

private slots:
  void openImage();
  void mdiSubWindowActivated(QMdiSubWindow *window);
};
