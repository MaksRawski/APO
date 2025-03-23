#pragma once

#include "histogramWidget.hpp"
#include <QLabel>
#include <QMainWindow>
#include <QScrollArea>
#include <qdockwidget.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qtabwidget.h>
#include <QMdiArea>

const int INITIAL_WIDTH = 400;
const int INITIAL_HEIGHT = 400;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);

private:
  QMdiArea *mdiArea;
  QDockWidget *dock;
  HistogramWidget *histogramWidget;

  // setup functions
  void setupMenuBar();
  void setupUI();

  // connections
  void openImage();
  void closeTab(int index);
};
