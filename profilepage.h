#ifndef PROFILEPAGE_H
#define PROFILEPAGE_H

#include <QWidget>
#include <QString>

namespace Ui {
class ProfilePage;
}

class ProfilePage : public QWidget
{
    Q_OBJECT

public:
    explicit ProfilePage(QWidget *parent = nullptr);
    ~ProfilePage();

    void setUserInfo(const QString &username, const QString &role);
    void clearUserInfo();
    bool hasUserInfo() const;

signals:
    void backToHomeRequested();

private:
    Ui::ProfilePage *ui;
    bool userInfoAvailable = false;
};

#endif // PROFILEPAGE_H
