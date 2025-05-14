#pragma once

#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <QLabel>
#include <QPixmap>
#include <QWheelEvent>
#include <qscrollarea.h>
#include <qtmetamacros.h>
#include <qwidget.h>

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
