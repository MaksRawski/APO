#include "histogramWidget.hpp"
#include "imageProcessor.hpp"
#include <algorithm>
#include <qpixmap.h>

HistogramWidget::HistogramWidget(QWidget *parent) : QWidget(parent) {
    setMinimumSize(256, 100);
}

void HistogramWidget::updateHistogram(QPixmap pixmap) {
    this->pixmap = pixmap;
    lut = imageProcessor::histogram(pixmap.toImage());
	maxLutValue = *std::max_element(std::begin(lut), std::end(lut));
}

void HistogramWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

	int lut_size = (int)lut.size();
	if (lut_size <= 0) return;

	int w = width();
	int h = height();
    int barWidth = w / lut_size;
    int maxHeight = h - 10;

    for (int i = 0; i < lut.size(); ++i) {
        int barHeight = int((float)lut[i] / (float)maxLutValue * (float)maxHeight);
        QPoint topLeft(i * barWidth, height() - barHeight);
        QPoint bottomRight((i + 1) * barWidth, height());

        painter.setPen(Qt::black);
        painter.drawLine(topLeft, bottomRight);

        painter.setBrush(Qt::gray);
        painter.drawRect(QRectF(topLeft, bottomRight));
    }
}
