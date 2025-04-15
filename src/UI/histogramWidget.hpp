#pragma once

#include "../imageProcessor.hpp"
#include <QImage>
#include <QPainter>
#include <QScrollArea>
#include <QSplitter>
#include <QWidget>
#include <qlabel.h>
#include <qlistwidget.h>
#include <qpixmap.h>
#include <qsplitter.h>
#include <qwidget.h>

class HistogramPlot : public QWidget {
  Q_OBJECT

public:
  explicit HistogramPlot(QWidget *parent = nullptr);

public slots:
  void updateHist(std::vector<int> hist, int max);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  std::vector<int> hist;
  int maxLutValue;
};

// Upper half contains the histogram, the bottom one stats
class HistogramWidget : public QSplitter {
  Q_OBJECT

public:
  explicit HistogramWidget(QWidget *parent = nullptr);
  void reset();

public slots:
  void updateHistogram(const ImageWrapper &image);

signals:
  void updateHist(std::vector<int> lut, int max);

private:
  HistogramPlot *plot;
  QLabel *statsLabel;
  QListWidget *lutList;

  std::vector<int> hist;
  int min, max;
  double average;
};
