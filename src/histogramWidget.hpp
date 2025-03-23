#pragma once

#include <QWidget>
#include <QPainter>
#include <QImage>
#include <qpixmap.h>
#include "imageProcessor.hpp"

class HistogramWidget : public QWidget {
    Q_OBJECT

public:
    explicit HistogramWidget(QWidget *parent = nullptr);

public slots:
    void update(const QPixmap pixmap);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap pixmap;
    imageProcessor::LUT lut;
};
