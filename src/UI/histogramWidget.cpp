#include "histogramWidget.hpp"
#include "../imageProcessor.hpp"
#include <QListWidget>
#include <qboxlayout.h>
#include <qbrush.h>
#include <qlabel.h>
#include <qnamespace.h>
#include <qpixmap.h>
#include <qscrollarea.h>
#include <qwidget.h>

HistogramPlot::HistogramPlot(QWidget *parent) : QWidget(parent) {
  setMinimumSize(256, 100);
  resize(400, 400);
}
void HistogramPlot::updateHist(std::vector<int> hist, int maxValue) {
  this->hist = hist;
  maxLutValue = maxValue;
  update();
}

void HistogramPlot::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  int lut_size = hist.size();
  if (lut_size <= 0)
    return;

  int w = width();
  int h = height();
  int gradientHeight = 50;
  int spacing = 20;
  int maxHeight = height() - gradientHeight;

  if (maxLutValue == 0)
    return;

  double barWidth =
      fmax(1.0f, static_cast<double>(w) / static_cast<double>(lut_size));

  for (int i = 0; i < lut_size; ++i) {
    int barHeight = static_cast<int>(
        (static_cast<double>(hist[i]) / maxLutValue) * maxHeight);
    int x = i * barWidth;
    int y = h - barHeight;

    painter.setPen(Qt::gray);
    painter.setBrush(Qt::gray);
    painter.drawRect(x, y, barWidth, barHeight);
  }

  // gradient
  QLinearGradient gradient(0, maxHeight + spacing, w,
                           maxHeight + spacing + gradientHeight);
  for (size_t i = 0; i < hist.size(); ++i) {
    double intensity = static_cast<double>(i) /
                       static_cast<double>(hist.size() - 1); // normalize
    double position = intensity;

    QColor color =
        QColor::fromRgbF(intensity, intensity, intensity); // grayscale

    gradient.setColorAt(position, color);
  }

  painter.fillRect(0, maxHeight + spacing, w, gradientHeight, gradient);
}

HistogramWidget::HistogramWidget(QWidget *parent)
    : QSplitter(Qt::Vertical, parent) {
  // PLOT
  plot = new HistogramPlot;

  QWidget *statsAndLut = new QWidget;
  QVBoxLayout *statsAndLutLayout = new QVBoxLayout(statsAndLut);

  // STATS
  statsLabel = new QLabel("Min: -- Max: -- Avg: --");
  statsAndLutLayout->addWidget(statsLabel);

  // LUT
  QScrollArea *lutScrollArea = new QScrollArea;
  lutScrollArea->setWidgetResizable(true);
  lutList = new QListWidget;
  lutScrollArea->setWidget(lutList);
  statsAndLutLayout->addWidget(lutScrollArea);

  addWidget(plot);
  addWidget(statsAndLut);

  connect(this, &HistogramWidget::updateHist, plot, &HistogramPlot::updateHist);
}

void HistogramWidget::updateHistogram(const ImageWrapper &image) {
  hist = imageProcessor::histogram(image);
  if (hist.empty()) {
    reset();
    return;
  }

  min = hist[0];
  max = hist[0];
  int sum = 0;
  lutList->clear();
  for (int i = 0; i < 256; ++i) {
    int l = hist[i];
    if (l < min)
      min = l;
    if (l > max)
      max = l;
    sum += l;
  }

  int minValue = -1;
  int maxValue = -1;
  for (int i = 0; i < 256; ++i) {
    int l = hist[i];
    if (l > 0 && minValue == -1)
      minValue = i;
    if (l > 0)
      maxValue = i;
    double percent = 0;
    if (l > 0) percent = floor((double)l / (double)max * 10000) / 100;
    lutList->addItem(QString("%1:\t%2\t%3%").arg(i).arg(l).arg(percent));
  }
  average = static_cast<double>(sum) / 256;

  statsLabel->setText(
      QString("Min value: %5\tMax value: %6\n"
              "Pixels: %1\tMin count: %2\tMax count: %3\tAvg: %4\n\n"
              "Value\tCount\tPercent of max count")
          .arg(sum)
          .arg(min)
          .arg(max)
          .arg(average, 0, 'f', 2)
          .arg(minValue)
          .arg(maxValue));

  emit updateHist(hist, max);
}

void HistogramWidget::reset() {
  hist.clear();
  lutList->clear();
  plot->updateHist(hist, 0);
  statsLabel->setText(QString("Min: -- Max: -- Avg: --"));
}
