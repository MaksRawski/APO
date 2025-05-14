#pragma once

#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <QLabel>
#include <QPixmap>
#include <QWheelEvent>
#include <qscrollarea.h>
#include <qtmetamacros.h>
#include <qwidget.h>

// class ZoomableImage : public QLabel {
//   Q_OBJECT

// public:
//   ZoomableImage(QWidget *parent = nullptr);

//   void setImage(const QPixmap &pixmap);
//   void setImageScale(double scale);
//   void clear();

//   double getImageScale() const;
//   QPixmap getImage() const;

// private:
//   QPixmap originalPixmap;
//   double scaleFactor;
// };

// class ImageViewerOld : public QScrollArea {
//   Q_OBJECT
// public:
//   ImageViewerOld(QWidget *parent = nullptr);
//   void setImage(const QPixmap &pixmap);
//   void setImageScale(double scale);
//   void clear();

//   double getImageScale() const;
//   QPixmap getImage() const;

// protected:
//   void wheelEvent(QWheelEvent *event) override;

// private:
//   ZoomableImage imageLabel;
// };

class ImageViewer : public QGraphicsView {
  Q_OBJECT
public:
  explicit ImageViewer(QWidget *parent = nullptr);
  void setImage(const QPixmap &pixmap);
  void useImageTransform(const ImageViewer &other);
  void fit();
  void clear();

  QPixmap getImage() const;

protected:
  void wheelEvent(QWheelEvent *event) override;

private:
  QGraphicsScene scene;
  QGraphicsPixmapItem *pixmapItem = nullptr;
  const qreal zoomFactor;
};
