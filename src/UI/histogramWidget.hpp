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
  void updateLUT(imageProcessor::LUT l, int max);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  imageProcessor::LUT lut;
  int maxLutValue;
};

// Upper half contains the histogram, the bottom one stats
class HistogramWidget : public QSplitter {
  Q_OBJECT

public:
  explicit HistogramWidget(QWidget *parent = nullptr);
  void reset();

public slots:
  void updateHistogram(QPixmap pixmap);

signals:
  void updateLUT(imageProcessor::LUT lut, int max);

private:
  HistogramPlot *plot;
  QLabel *statsLabel;
  QListWidget *lutList;

  imageProcessor::LUT lut;
  int min, max;
  float average;
};
