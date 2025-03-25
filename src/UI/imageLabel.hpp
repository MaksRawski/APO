#pragma once

#include <QLabel>
#include <QPixmap>
#include <QWheelEvent>

// Container for a zoomable image
class ImageLabel : public QLabel {
  Q_OBJECT

public:
  explicit ImageLabel(QWidget *parent = nullptr);

  void setImage(const QPixmap &pixmap);
  void setImageScale(double scale);

  double getImageScale() const;
  QPixmap getImage() const;

protected:
  void wheelEvent(QWheelEvent *event) override;

private:
  QPixmap originalPixmap;
  double scaleFactor;
};
