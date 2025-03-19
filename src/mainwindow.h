#pragma once

#include <QMainWindow>
#include <qlabel.h>
#include <qscrollarea.h>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void openImage();

private:
    QLabel *imageLabel;
    QScrollArea *scrollArea;
};
