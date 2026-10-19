// #include "registerpage.h"
// #include <QApplication>

// int main(int argc, char *argv[])
// {
//     QApplication app(argc, argv);

//     RegisterPage page;
//     page.show();

//     return app.exec();
// }


#include "profilepage.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ProfilePage page;
    page.setUserInfo("testuser", "Student");
    page.show();

    return app.exec();
}


// #include "profilepage.h"
// #include <QApplication>
// #include <QDebug>

// int main(int argc, char *argv[])
// {
//     QApplication app(argc, argv);

//     ProfilePage page;
//     page.setUserInfo("testuser", "Student");

//     QObject::connect(&page, &ProfilePage::backToHomeRequested,
//                      []() {
//                          qDebug() << "Back to Home button clicked!";
//                      });

//     page.show();

//     return app.exec();
// }