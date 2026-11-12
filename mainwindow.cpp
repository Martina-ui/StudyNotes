#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->profileButton, &QPushButton::clicked,
            this, &MainWindow::profileRequested);

    connect(ui->createNoteButton, &QPushButton::clicked,
            this, &MainWindow::createNoteRequested);

    connect(ui->myNotesButton, &QPushButton::clicked,
            this, &MainWindow::myNotesRequested);

    connect(ui->publicNotesButton, &QPushButton::clicked,
            this, &MainWindow::publicNotesRequested);

    connect(ui->adminPanelButton, &QPushButton::clicked,
            this, &MainWindow::adminPanelRequested);

    connect(ui->logoutButton, &QPushButton::clicked,
            this, &MainWindow::logoutRequested);

    setAdminVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setAdminVisible(bool visible)
{
    ui->adminPanelButton->setVisible(visible);
}
