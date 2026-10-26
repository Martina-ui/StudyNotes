#ifndef MYNOTESPAGE_H
#define MYNOTESPAGE_H

#include <QWidget>
#include <QString>
#include <QList>

struct NoteSummary
{
    int noteId;
    QString title;
    QString category;
    QString content;
    QString visibility;
};

QT_BEGIN_NAMESPACE
namespace Ui { class MyNotesPage; }
QT_END_NAMESPACE

class MyNotesPage : public QWidget
{
    Q_OBJECT

public:
    explicit MyNotesPage(QWidget *parent = nullptr);
    ~MyNotesPage() override;

    void setNotes(const QList<NoteSummary> &notes);
    void clearNotes();
    void showError(const QString &message);

signals:
    void createNoteRequested();
    void editNoteRequested(int noteId);
    void deleteNoteRequested(int noteId);
    void backRequested();

private slots:
    void onNoteSelected(int row);
    void onEditClicked();
    void onDeleteClicked();

private:
    Ui::MyNotesPage *ui;
    QList<NoteSummary> currentNotes;
    void updateSelection();
};

#endif // MYNOTESPAGE_H
