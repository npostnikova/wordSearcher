#include "mainwindow.h"
#include "setmanager.h"

#include <QCommonStyle>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QFuture>
#include <QtConcurrent/QtConcurrentRun>
#include <QDesktopWidget>
#include <QProcess>
#include <QTimer>
#include <QObject>
#include <QMessageBox>
#include <QDesktopServices>

#include "filehighlighterdialog.h"
#include "ui_filehighlighterdialog.h"

#include <QFutureWatcher>
#include <QTextStream>


#include "directoryindexer.h"

void MainWindow::openFile(const QModelIndex& index) {
    if (index.column() == 0 && ui->duplicatesTreeView->getModel()->itemFromIndex(index)->rowCount() == 0) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(index.data().toString()));
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), thread(new QThread), searcher(new WordSearcher), searcherThread(new QThread) {
    ui->setupUi(this);

    searcher->moveToThread(searcherThread.get());


    connect(ui->duplicatesTreeView,
                &MyTreeView::activated,
                this,
                &MainWindow::openWithHighlight);

    connect(ui->actionScan_Directory, &QAction::triggered, this, &MainWindow::selectDirectory);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::quit);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);
    connect(ui->actionCancel, &QAction::triggered, this, &MainWindow::cancelSearching);
    connect(ui->lineEdit, SIGNAL(textEdited(const QString &)), this, SLOT(findWord(const QString &)));
    connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(findWord()));


    //connect(this, SIGNAL(closed()), this, SLOT(cancelSearching()));

    connect(searcher.get(), SIGNAL(error(Message const&)), this, SLOT(addError(Message const&)));
    connect(searcher.get(), SIGNAL(finishedSearching()), this, SLOT(finishedSearching()));
    qRegisterMetaType<std::unordered_set<size_t>>("std::unordered_set<size_t>");
    qRegisterMetaType<size_t>("size_t");
    qRegisterMetaType<std::vector<QString>>("std::vector<QString>");

    connect(searcher.get(), SIGNAL(sendFiles(std::vector<QString>)), this, SLOT(insertInTree(std::vector<QString>)));

    connect(this, SIGNAL(find(QString const&, QString const&, std::unordered_map<uint32_t, std::unordered_set<size_t>> * ,
                              std::map<size_t, QString> * )), searcher.get(), SLOT(runSearcher(QString const&, QString const&, std::unordered_map<uint32_t, std::unordered_set<size_t>> * ,
                                                                                         std::map<size_t, QString> * )));
    connect(this, SIGNAL(interruptSearching()), searcher.get(), SLOT(cancel()), Qt::DirectConnection);

}


void MainWindow::openWithHighlight(const QModelIndex& i) {
    QModelIndex index(i.column() == 0 ? i : i.siblingAtColumn(0));
    dialog = std::unique_ptr<FileHighlighterDialog>(new FileHighlighterDialog(ui->lineEdit->text(), QDir(curDir).filePath(index.data().toString())));

    dialog->setText();
    dialog->setWindowFlags(Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    dialog->setWindowTitle(index.data().toString());
    dialog->show();
}

MainWindow::~MainWindow() {
    if (dialog)
        dialog->close();
    dialog.reset(nullptr);
    cancelSearching();
    thread->quit();
    thread->wait();
    searcherThread->quit();
    searcherThread->wait();
}


void MainWindow::selectDirectory() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Indexing",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir != nullptr) {
        ui->searchAction(ui->START);
        trigrams.clear();
        scanDirectory(dir);
        ui->searchAction(ui->FINISH);
    }
}


void MainWindow::scanDirectory(QString const& dir) {
    curDir = dir;
    ui->duplicatesTreeView->clean();
    ui->exceptionsList->clean();
    cancelled = false;
    setWindowTitle(QString("Word Searcher Directory - %1").arg(dir));
    QThread::currentThread()->setPriority(QThread::HighPriority);

    indexDirectory(dir);

}

void MainWindow::showAboutDialog() {
    QMessageBox::aboutQt(this);
}

void MainWindow::cancelSearching() {
    if (thread->isRunning()) {
        cancelled = true;
        emit interruptSearching();
    } else if (searcherThread->isRunning()) {
        emit interruptSearching();
        while (!searcher->wasCancelled()) {;}
    }
}

void MainWindow::quit() {
    cancelled = true;
    cancelSearching();
    QWidget::close();
}

void MainWindow::deleteFile(QString name) {
    QFile file(name);
    if (file.exists()) {
        ui->exceptionsList->insertError({name, Message::DONE, "Deleted: "});
    } else {
        ui->exceptionsList->insertError({name, Message::ERROR, "Cannot delete the file"});
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
     cancelSearching();
}

void MainWindow::indexDirectory(QString dirName) {
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<std::set<uint32_t>>("std::set<uint32_t>"); // not here?

    DirectoryIndexer * indexer = new DirectoryIndexer(dirName, thread.get());

    connect(this, SIGNAL(interruptSearching()), indexer, SLOT(cancelSearching()), Qt::DirectConnection);
    connect(indexer, SIGNAL(sendError(Message)), this, SLOT(addError(Message)));
    QFuture<void> future = QtConcurrent::run(DirectoryIndexer::getTrigrams, indexer, &fileNames, &trigrams);
    QTimer timer;

    setAttribute( Qt::WA_DeleteOnClose );
    while (!future.isFinished()) {
        if (cancelled) {
            indexer->cancelSearching();
        }
        timer.start(1000);
        ui->progressBar->setMaximum(indexer->getDirSize());
        ui->progressBar->setValue(indexer->getProgress());
        ui->statusLabel->setText(indexer->getDirSize() == 0? "Indexing directory..." : "Progress: " + QString::number(indexer->getProgress()) + "/" + QString::number(indexer->getDirSize()));
        QApplication::processEvents(QEventLoop::AllEvents  | QEventLoop::WaitForMoreEvents);
    }
    ui->statusLabel->setText("Done: " + QString::number(indexer->getProgress()) + "/" + QString::number(indexer->getDirSize()));
    cancelled = false;
    thread->quit();
    thread->wait();
    delete indexer;
}

void MainWindow::findWord(const QString & word) {
    if (ui->lineEdit->text().isEmpty()) {
        ui->duplicatesTreeView->clean();
        return;
    }
    if (searcherThread->isRunning()) {
        emit interruptSearching();


        while (!searcher->wasCancelled()) {;
        }
    }

    ui->actionCancel->setEnabled(true);
    ui->duplicatesTreeView->clean();
    cancelled = false;


    searcherThread->start();
    emit find(word == nullptr ? ui->lineEdit->text() : word, curDir, &trigrams, &fileNames);

}
