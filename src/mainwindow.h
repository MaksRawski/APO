#pragma once

#include "tab.h"
#include <QLabel>
#include <QMainWindow>
#include <QScrollArea>
#include <qimage.h>
#include <qpixmap.h>
#include <qtabwidget.h>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);

private:
  std::vector<Tab> tabs;
  QTabWidget *tabWidget;

  // setup functions
  void setupMenuBar();

  // connections
  void openImage();
  void closeTab(int index);
};
