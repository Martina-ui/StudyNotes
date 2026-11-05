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

    connect(ui->logoutButton, &QPushButton::clicked, //first time doing UI, I didn't match the style at all
            this, &MainWindow::logoutRequested);    //check that logout button is fully opperational first
}                                                  //That's basically my next job, with the testing and whatnot.

MainWindow::~MainWindow()
{
    delete ui;
}
