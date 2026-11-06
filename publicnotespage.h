#ifndef PUBLICNOTESPAGE_H
#define PUBLICNOTESPAGE_H

#include <QWidget>
#include <QList>
#include <QString>
#include "mynotespage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PublicNotesPage; }
QT_END_NAMESPACE

class PublicNotesPage : public QWidget
{
    Q_OBJECT

public:
    explicit PublicNotesPage(QWidget *parent = nullptr);
    ~PublicNotesPage() override;
    void setNotes(const QList<NoteSummary> &notes);
    void clearNotes();
    void showError(const QString &message);

signals:
    void backRequested();

private slots:
    void onNoteSelected(int row);
    void onSortChanged(int index);

private:
    Ui::PublicNotesPage *ui;
    QList<NoteSummary> currentNotes;
    void displayNotes();
    void updateSelection();
};

#endif // PUBLICNOTESPAGE_H
