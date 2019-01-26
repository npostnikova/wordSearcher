#ifndef FILEHIGHLIGHTERDIALOG_H
#define FILEHIGHLIGHTERDIALOG_H

#include <QDialog>

namespace Ui {
    class FileHighlighterDialog;
}

class FileHighlighterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileHighlighterDialog(QString word, QString fileName, QWidget *parent = nullptr);
    ~FileHighlighterDialog();

    void setText();

    QString fileName;

//private:
    Ui::FileHighlighterDialog *ui;

    friend struct MainWindow;
};

#endif // FILEHIGHLIGHTERDIALOG_H
