#include "noteentrypage.h"
#include "ui_noteentrypage.h"

#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

NoteEntryPage::NoteEntryPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NoteEntryPage)
{
    ui->setupUi(this);
    ui->titleLineEdit->setMaxLength(200);
    ui->visibilityComboBox->addItems({"Private", "Public"});
    ui->categoryComboBox->addItems({
        "Computer Science",
        "Mathematics",
        "Science",
        "English",
        "History",
        "Other"
    });

    connect(ui->createNoteButton, &QPushButton::clicked,
            this, &NoteEntryPage::onCreateNoteClicked);

    connect(ui->cancelButton, &QPushButton::clicked,
            this, &NoteEntryPage::onCancelClicked);

    ui->statusLabel->clear();
}

NoteEntryPage::~NoteEntryPage()
{
    delete ui;
}

void NoteEntryPage::onCreateNoteClicked()
{
    QString title = ui->titleLineEdit->text().trimmed();
    QString content = ui->contentTextEdit->toPlainText().trimmed();
    QString category = ui->categoryComboBox->currentText();
    QString visibility = ui->visibilityComboBox->currentText();

    if (title.isEmpty()) {
        showMessage("Please enter a note title.");
        return;
    }

    if (content.isEmpty()) {
        showMessage("Please enter note content.");
        return;
    }

    ui->statusLabel->clear();

    emit noteCreationRequested(title, content, category, visibility);
}

void NoteEntryPage::onCancelClicked()
{
    clearFields();
    emit cancelRequested();
}

void NoteEntryPage::showSaveSuccess()
{
    showMessage("Note created successfully!", true);
    clearFields();
}

void NoteEntryPage::showSaveFailure(const QString &message)
{
    showMessage(message.isEmpty() ? "Unable to create note." : message);
}

void NoteEntryPage::clearFields()
{
    ui->titleLineEdit->clear();
    ui->contentTextEdit->clear();
    ui->categoryComboBox->setCurrentIndex(0);
    ui->visibilityComboBox->setCurrentIndex(0);
}

void NoteEntryPage::showMessage(const QString &message, bool success)
{
    ui->statusLabel->setText(message);
    ui->statusLabel->setStyleSheet(
        success ? "color: #16803c;" : "color: #c0392b;"
        );
}
