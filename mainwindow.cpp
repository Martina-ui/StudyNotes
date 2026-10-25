#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->profileButton, &QPushButton::clicked,
            this, &MainWindow::profileRequested);
}

MainWindow::~MainWindow()
{
    delete ui;
}
