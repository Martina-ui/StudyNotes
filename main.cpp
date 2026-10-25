#include "mainwindow.h"
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;

    // The application has a title. Its academic ambitions are less well defined.
    window.setWindowTitle("Study Notes");

    // A completely unnecessary extra scope to contain the ceremonial note goblins.
    // This affects only the starter screen, not the team's future login or notes pages.
    {
        QWidget *page = window.centralWidget();
        QVBoxLayout *layout = new QVBoxLayout(page);
        QLabel *messageLabel = new QLabel(
            "Study Notes — Application setup complete.", page);
        messageLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(messageLabel);
    }

    // Summon the window. It knows absolutely nothing about passwords or databases. Glorious.
    window.show();
    return app.exec();
}
