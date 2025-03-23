#include "histogramWidget.hpp"
#include "../imageProcessor.hpp"
#include <QListWidget>
#include <algorithm>
#include <qlabel.h>
#include <qnamespace.h>
#include <qpixmap.h>
#include <qscrollarea.h>

HistogramPlot::HistogramPlot(QWidget *parent) : QWidget(parent) {
  setMinimumSize(256, 100);
  resize(400, 400);
}
void HistogramPlot::updateLUT(imageProcessor::LUT l, int maxValue) {
  lut = l;
  maxLutValue = maxValue;
  update();
}

void HistogramPlot::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  int lut_size = lut.size();
  if (lut_size <= 0)
    return;

  int w = width();
  int h = height();
  int maxHeight = h - 10;

  int maxLutValue = *std::max_element(lut.begin(), lut.end());

  if (maxLutValue == 0)
    return;

  int barWidth = std::max(1, w / lut_size);

  for (int i = 0; i < lut_size; ++i) {
    int barHeight = static_cast<int>(
        (static_cast<float>(lut[i]) / maxLutValue) * maxHeight);
    int x = i * barWidth;
    int y = h - barHeight;

    painter.setPen(Qt::black);
    painter.setBrush(Qt::gray);
    painter.drawRect(x, y, barWidth, barHeight);
  }
}

HistogramWidget::HistogramWidget(QWidget *parent)
    : QSplitter(Qt::Vertical, parent) {
  // PLOT
  plot = new HistogramPlot;

  // LUT
  QScrollArea *lutScrollArea = new QScrollArea;
  lutScrollArea->setWidgetResizable(true);
  lutList = new QListWidget;
  lutScrollArea->setWidget(lutList);

  // STATS
  statsLabel = new QLabel("Min: -- Max: -- Avg: --");

  addWidget(plot);
  addWidget(statsLabel);
  addWidget(lutScrollArea);

  connect(this, &HistogramWidget::updateLUT, plot, &HistogramPlot::updateLUT);
}

void HistogramWidget::updateHistogram(QPixmap pixmap) {
  lut = imageProcessor::histogram(pixmap.toImage());
  min = lut[0];
  max = lut[0];

  int sum = 0;
  for (int i = 0; i < lut.size(); ++i) {
    int l = lut[i];
    if (l < min)
      min = l;
    if (l > max)
      max = l;
    sum += l;

    // NOTE: if this was addItem was done in another loop __maybe__ the above
    // could use some compiler optimization magic?
    lutList->addItem(QString("%1: %2").arg(i).arg(l));
  }
  average = static_cast<float>(sum) / lut.size();

  statsLabel->setText(QString("Min: %1  Max: %2  Avg: %3")
                          .arg(min)
                          .arg(max)
                          .arg(average, 0, 'f', 2));

  emit updateLUT(lut, max);
}
