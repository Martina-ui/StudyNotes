#include "publicnotespage.h"
#include "ui_publicnotespage.h"

#include <QListWidget>
#include <QPushButton>
#include <QComboBox>
#include <algorithm>

PublicNotesPage::PublicNotesPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::PublicNotesPage)
{
    ui->setupUi(this);

    connect(ui->notesListWidget, &QListWidget::currentRowChanged,
            this, &PublicNotesPage::onNoteSelected);

    connect(ui->sortComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PublicNotesPage::onSortChanged);

    connect(ui->backButton, &QPushButton::clicked,
            this, &PublicNotesPage::backRequested);

    clearNotes();
}

PublicNotesPage::~PublicNotesPage()
{
    delete ui;
}

void PublicNotesPage::setNotes(const QList<NoteSummary> &notes)
{
    currentNotes = notes;
    ui->statusLabel->clear();
    displayNotes();
}

void PublicNotesPage::clearNotes()
{
    setNotes({});
}

void PublicNotesPage::showError(const QString &message)
{
    ui->statusLabel->setText(message);
}

void PublicNotesPage::onNoteSelected(int)
{
    updateSelection();
}

void PublicNotesPage::onSortChanged(int)
{
    displayNotes();
}

void PublicNotesPage::displayNotes()
{
    int sortOption = ui->sortComboBox->currentIndex();

    if (sortOption == 1) {
        std::sort(currentNotes.begin(), currentNotes.end(),
                  [](const NoteSummary &a, const NoteSummary &b) {
                      int categoryComparison = QString::compare(
                          a.category, b.category, Qt::CaseInsensitive);
                      if (categoryComparison == 0) {
                          return QString::compare(
                                     a.title, b.title, Qt::CaseInsensitive) < 0;
                      }

                      return categoryComparison < 0;
                  });
    } else if (sortOption == 2) {
        std::sort(currentNotes.begin(), currentNotes.end(),
                  [](const NoteSummary &a, const NoteSummary &b) {
                      return QString::compare(
                                 a.title, b.title, Qt::CaseInsensitive) < 0;
                  });
    } else {
        std::sort(currentNotes.begin(), currentNotes.end(),
                  [](const NoteSummary &a, const NoteSummary &b) {
                      return a.noteId > b.noteId;
                  });
    }

    ui->notesListWidget->clear();
    for (const NoteSummary &note : currentNotes) {
        ui->notesListWidget->addItem(note.title + "\n" + note.category);
    }
    if (currentNotes.isEmpty()) {
        ui->emptyLabel->setText("No public notes are available yet.");
        ui->emptyLabel->show();
    } else {
        ui->emptyLabel->hide();
        ui->notesListWidget->setCurrentRow(0);
    }
    updateSelection();
}

void PublicNotesPage::updateSelection()
{
    int row = ui->notesListWidget->currentRow();
    bool hasSelection = row >= 0 && row < currentNotes.size();
    if (!hasSelection) {
        ui->noteTitleValue->setText("Select a public note to read");
        ui->categoryValue->clear();
        ui->contentTextEdit->clear();
        return;
    }
    const NoteSummary &note = currentNotes.at(row);
    ui->noteTitleValue->setText(note.title);
    ui->categoryValue->setText("Course / Category: " + note.category);
    ui->contentTextEdit->setPlainText(note.content);
}
