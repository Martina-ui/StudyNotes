#include "adminpanelpage.h"
#include "ui_adminpanelpage.h"

#include <QListWidget>
#include <QPushButton>

AdminPanelPage::AdminPanelPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::AdminPanelPage)
{
    ui->setupUi(this);
    connect(ui->notesListWidget, &QListWidget::currentRowChanged,
            this, &AdminPanelPage::onNoteSelected);

    connect(ui->backButton, &QPushButton::clicked,
            this, &AdminPanelPage::backRequested);
    connect(ui->deleteButton, &QPushButton::clicked,
            this, &AdminPanelPage::onDeleteClicked);
    clearNotes();
}

AdminPanelPage::~AdminPanelPage()
{
    delete ui;
}

void AdminPanelPage::setNotes(const QList<NoteSummary> &notes)
{
    currentNotes = notes;
    ui->notesListWidget->clear();
    ui->statusLabel->clear();
    for (const NoteSummary &note : currentNotes) {
        ui->notesListWidget->addItem(note.title + "\n" + note.category);
    }
    if (currentNotes.isEmpty()) {
        ui->emptyLabel->setText("No public notes are available for moderation.");
        ui->emptyLabel->show();
    } else {
        ui->emptyLabel->hide();
        ui->notesListWidget->setCurrentRow(0);
    }
    updateSelection();
}

void AdminPanelPage::clearNotes()
{
    setNotes({});
}

void AdminPanelPage::showError(const QString &message)
{
    ui->statusLabel->setText(message);
}

void AdminPanelPage::onNoteSelected(int)
{
    updateSelection();
}

void AdminPanelPage::onDeleteClicked()
{
    int row = ui->notesListWidget->currentRow();
    if (row < 0 || row >= currentNotes.size()) {
        return;
    }
    emit deleteNoteRequested(currentNotes.at(row).noteId);
}

void AdminPanelPage::updateSelection()
{
    int row = ui->notesListWidget->currentRow();
    bool hasSelection = row >= 0 && row < currentNotes.size();
    //deletion will be enabled when Issue #15 is implemented.
    ui->deleteButton->setEnabled(false);

    if (!hasSelection) {
        ui->noteTitleValue->setText("Select a public note to review");
        ui->categoryValue->clear();
        ui->visibilityValue->clear();
        ui->contentTextEdit->clear();
        return;
    }
    const NoteSummary &note = currentNotes.at(row);
    ui->noteTitleValue->setText(note.title);
    ui->categoryValue->setText("Course / Category: " + note.category);
    ui->visibilityValue->setText("Visibility: " + note.visibility);
    ui->contentTextEdit->setPlainText(note.content);
}
