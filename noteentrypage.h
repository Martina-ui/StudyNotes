#ifndef NOTEENTRYPAGE_H
#define NOTEENTRYPAGE_H

#include <QWidget>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui {
class NoteEntryPage;
}
QT_END_NAMESPACE

class NoteEntryPage : public QWidget
{
    Q_OBJECT

public:
    explicit NoteEntryPage(QWidget *parent = nullptr);
    ~NoteEntryPage() override;

    void showSaveSuccess();
    void showSaveFailure(const QString &message);
    void clearFields();

signals:
    void noteCreationRequested(const QString &title,
                               const QString &content,
                               const QString &category,
                               const QString &visibility);

    void cancelRequested();

private slots:
    void onCreateNoteClicked();
    void onCancelClicked();

private:
    Ui::NoteEntryPage *ui;

    void showMessage(const QString &message, bool success = false);
};

#endif // NOTEENTRYPAGE_H
