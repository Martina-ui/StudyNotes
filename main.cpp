#include "registerpage.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    RegisterPage page;
    page.show();

    return app.exec();
}
