#include "loginpage.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LoginPage page;
    page.show();

    return app.exec();
}