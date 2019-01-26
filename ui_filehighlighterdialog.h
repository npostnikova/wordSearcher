#ifndef UI_FILEHIGHLIGHTERDIALOG_H
#define UI_FILEHIGHLIGHTERDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QTextBrowser>
#include <QHBoxLayout>
#include "highlighter.h"

QT_BEGIN_NAMESPACE

class Ui_FileHighlighterDialog
{
public:
    QTextBrowser *textBrowser;
    Highlighter * highlighter;
    QHBoxLayout * layout;

    void setupUi(QString word, QString fileName, QDialog *FileHighlighterDialog)
    {
        if (FileHighlighterDialog->objectName().isEmpty())
            FileHighlighterDialog->setObjectName(QStringLiteral("FileHighlighterDialog"));
        FileHighlighterDialog->resize(715, 544);


        layout = new QHBoxLayout(FileHighlighterDialog);
        //textBrowser = new QTextBrowser(FileHighlighterDialog);
        setUpTextBrowser();

        highlighter = new Highlighter(word, textBrowser->document());

        layout->addWidget(textBrowser);
        layout->setMargin(0);
        QFile file;
        file.setFileName(fileName);
        file.open(QFile::ReadOnly);
        textBrowser->setPlainText(file.readAll());
        textBrowser->setObjectName(QStringLiteral("textBrowser"));


        retranslateUi(FileHighlighterDialog);

        QMetaObject::connectSlotsByName(FileHighlighterDialog);

    } // setupUi

    void retranslateUi(QDialog *FileHighlighterDialog)
    {
        FileHighlighterDialog->setWindowTitle(QApplication::translate("FileHighlighterDialog", "Dialog", nullptr));
    } // retranslateUi

    void setUpTextBrowser() {

        QFont font;
        font.setFamily("Courier");
        font.setFixedPitch(true);
        font.setPointSize(10);

        textBrowser = new QTextBrowser;
        textBrowser->setFont(font);
    }

};

namespace Ui {
    class FileHighlighterDialog: public Ui_FileHighlighterDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILEHIGHLIGHTERDIALOG_H
