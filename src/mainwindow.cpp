#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Connect button signal to slot
    connect(ui->pushButton, &QPushButton::clicked, []() {
        qDebug() << "Button Clicked!";
    });
}

MainWindow::~MainWindow() {
    delete ui;
}
