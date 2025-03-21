#pragma once

#include "tab.hpp"
#include <QLabel>
#include <QMainWindow>
#include <QScrollArea>
#include <qimage.h>
#include <qpixmap.h>
#include <qtabwidget.h>

const int INITIAL_WIDTH = 400;
const int INITIAL_HEIGHT = 400;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);

private:
  // keeping their references so that the objects don't drop
  std::vector<Tab> tabs;
  QTabWidget *tabWidget;

  // setup functions
  void setupMenuBar();
  void setupTabs();

  // connections
  void openImage();
  void closeTab(int index);
};
