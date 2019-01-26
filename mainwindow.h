#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <memory>
#include "wordsearcher.h"
#include "ui_mainwindow.h"
#include "filehighlighterdialog.h"
#include "mytreeview.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:

    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

signals:
    void interruptSearching();
    void find(QString const& word, QString const& dir, std::unordered_map<uint32_t, std::unordered_set<size_t>> * blocks,
              std::map<size_t, QString> * fileNames);

private slots:
    void openFile(const QModelIndex& index);

    void addError(Message const& m) {
        ui->exceptionsList->insertError(m);
    }

    void openWithHighlight(const QModelIndex& index);

    void selectDirectory();

    void scanDirectory(QString const& dir);

    void showAboutDialog();

    void cancelSearching();

    void quit();

    void finishedSearching() {
        ui->actionCancel->setEnabled(false);
        searcherThread->quit();
        searcherThread->wait();

    }

    void insertInTree(std::vector<QString> files) {
        ui->duplicatesTreeView->getModel()->appendRows(files);
        QApplication::processEvents(QEventLoop::AllEvents);
    }

    void deleteFile(QString name);

    void findWord(const QString & word = nullptr);

protected:
     void closeEvent(QCloseEvent *event);


private:

    std::unique_ptr<QThread> thread;

    void indexDirectory(QString dirName);

    std::unique_ptr<Ui::MainWindow> ui;

    std::unique_ptr<FileHighlighterDialog> dialog;

    std::unordered_map<uint32_t, std::unordered_set<size_t>> trigrams;

    std::map<size_t, QString> fileNames;

    bool cancelled = false;

    std::unique_ptr<WordSearcher> searcher;


    std::unique_ptr<QThread> searcherThread;// = nullptr;

    QString curDir = nullptr;
};

#endif // MAINWINDOW_H
