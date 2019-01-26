#include "filehighlighterdialog.h"
#include "ui_filehighlighterdialog.h"

FileHighlighterDialog::FileHighlighterDialog(QString word, QString fileName, QWidget *parent) :
    QDialog(parent), fileName(fileName),
    ui(new Ui::FileHighlighterDialog)
{
    ui->setupUi(word, fileName, this);
}

FileHighlighterDialog::~FileHighlighterDialog()
{
    delete ui;
}

void FileHighlighterDialog::setText() {
    QFile file;
    file.setFileName(fileName);
    if (file.open(QFile::ReadOnly)) {
        ui->textBrowser->setPlainText(file.readAll());
    }
}

