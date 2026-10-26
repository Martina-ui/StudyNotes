#include "mynotespage.h"
#include "ui_mynotespage.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>

MyNotesPage::MyNotesPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::MyNotesPage)
{
    ui->setupUi(this);

    connect(ui->notesListWidget, &QListWidget::currentRowChanged,
            this, &MyNotesPage::onNoteSelected);
    connect(ui->createNoteButton, &QPushButton::clicked,
            this, &MyNotesPage::createNoteRequested);
    connect(ui->backButton, &QPushButton::clicked,
            this, &MyNotesPage::backRequested);
    connect(ui->editButton, &QPushButton::clicked,
            this, &MyNotesPage::onEditClicked);
    connect(ui->deleteButton, &QPushButton::clicked,
            this, &MyNotesPage::onDeleteClicked);

    clearNotes();
}

MyNotesPage::~MyNotesPage()
{
    delete ui;
}

void MyNotesPage::setNotes(const QList<NoteSummary> &notes)
{
    currentNotes = notes;
    ui->notesListWidget->clear();
    ui->statusLabel->clear();

    for (const NoteSummary &note : currentNotes) {
        ui->notesListWidget->addItem(note.title + "\n" + note.category);
    }

    if (currentNotes.isEmpty()) {
        ui->emptyLabel->setText("You don't have any notes yet. Create your first note!");
        ui->emptyLabel->show();
    } else {
        ui->emptyLabel->hide();
        ui->notesListWidget->setCurrentRow(0);
    }

    updateSelection();
}

void MyNotesPage::clearNotes()
{
    setNotes({});
}

void MyNotesPage::showError(const QString &message)
{
    ui->statusLabel->setText(message);
}

void MyNotesPage::onNoteSelected(int)
{
    updateSelection();
}

void MyNotesPage::updateSelection()
{
    int row = ui->notesListWidget->currentRow();
    bool hasSelection = row >= 0 && row < currentNotes.size();

    ui->editButton->setEnabled(hasSelection);
    ui->deleteButton->setEnabled(hasSelection);

    if (!hasSelection) {
        ui->noteTitleValue->setText("Select a note to view its details");
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

void MyNotesPage::onEditClicked()
{
    int row = ui->notesListWidget->currentRow();
    if (row >= 0 && row < currentNotes.size()) {
        emit editNoteRequested(currentNotes.at(row).noteId);
    }
}

void MyNotesPage::onDeleteClicked()
{
    int row = ui->notesListWidget->currentRow();
    if (row >= 0 && row < currentNotes.size()) {
        emit deleteNoteRequested(currentNotes.at(row).noteId);
    }
}
