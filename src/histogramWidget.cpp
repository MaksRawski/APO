#include "histogramWidget.hpp"
#include "imageProcessor.hpp"
#include <qpixmap.h>

HistogramWidget::HistogramWidget(QWidget *parent) : QWidget(parent) {
    setMinimumSize(200, 100);
}

void HistogramWidget::update(QPixmap pixmap) {
	qDebug() << "histogramWidget: update";
    this->pixmap = pixmap;
    lut = imageProcessor::histogram(pixmap.toImage());
    update(pixmap);
}

void HistogramWidget::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
	qDebug() << "paintEvent";

	int lut_size = (int)lut.size();
	if (lut_size <= 0) return;

	int w = pixmap.width();
	int h = pixmap.height();
	int numOfPixels = w * h;

    int barWidth = width() / lut_size;
    int maxHeight = height() - 10;

	// taking a reciprocal here to multiply instead of dividing later in the loop
	double maxLutHeight = 1.0 / (static_cast<double>(numOfPixels) * maxHeight);

    for (int i = 0; i < lut.size(); ++i) {
        int barHeight = double(lut[i]) * maxLutHeight;
        QPoint topLeft(i * barWidth, height() - barHeight);
        QPoint bottomRight((i + 1) * barWidth, height());

        painter.setPen(Qt::black);
        painter.drawLine(topLeft, bottomRight);

        painter.setBrush(Qt::gray);
        painter.drawRect(QRectF(topLeft, bottomRight));
    }
}
