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

    connect(ui->createNoteButton, &QPushButton::clicked,
            this, &MainWindow::createNoteRequested);

    connect(ui->myNotesButton, &QPushButton::clicked,
            this, &MainWindow::myNotesRequested);
}

MainWindow::~MainWindow()
{
    delete ui;
}
