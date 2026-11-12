#ifndef ADMINPANELPAGE_H
#define ADMINPANELPAGE_H

#include <QWidget>
#include <QList>
#include <QString>
#include "mynotespage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class AdminPanelPage; }
QT_END_NAMESPACE

class AdminPanelPage : public QWidget
{
    Q_OBJECT

public:
    explicit AdminPanelPage(QWidget *parent = nullptr);
    ~AdminPanelPage() override;
    void setNotes(const QList<NoteSummary> &notes);
    void clearNotes();
    void showError(const QString &message);

signals:
    void backRequested();
    void deleteNoteRequested(int noteId);

private slots:
    void onNoteSelected(int row);
    void onDeleteClicked();

private:
    Ui::AdminPanelPage *ui;
    QList<NoteSummary> currentNotes;
    void updateSelection();
};

#endif
