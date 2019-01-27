#ifndef DIRECTORYINDEXER_H
#define DIRECTORYINDEXER_H

#include <QString>
#include <memory>
#include <QThread>
#include <QObject>
#include <QDir>
#include <map>
#include <atomic>
#include <QTimer>
#include <QMimeDatabase>

#include <mutex>
#include <condition_variable>

#include "setmanager.h"
#include "trigramsearcher.h"
#include "mylistview.h"

struct DirectoryIndexer : QObject {
    Q_OBJECT
public:
    DirectoryIndexer(QString dirName, QThread * thread) : dirName(dirName), thread(thread), manager(new SetManager) {

        manager->moveToThread(thread);

        for (size_t i = 0; i < threadsNumber; i++) {
            threads.push_back(std::make_shared<QThread>());
            searchers.push_back(std::make_shared<TrigramSearcher>(i));
            searchers[i]->moveToThread(threads[i].get());
        }

        QObject::connect(this, SIGNAL(s1(QString, size_t)), searchers[0].get(), SLOT(insertFileSlot(QString, size_t)), Qt::QueuedConnection);
        QObject::connect(this, SIGNAL(s2(QString, size_t)), searchers[1].get(), SLOT(insertFileSlot(QString, size_t)), Qt::QueuedConnection);

        for (auto s : searchers) {
            QObject::connect(s.get(), SIGNAL(fileSet(size_t, std::set<uint32_t>)), manager.get(), SLOT(updateSet(size_t, std::set<uint32_t>)), Qt::QueuedConnection);
            QObject::connect(s.get(), SIGNAL(finishedSet(size_t)), this, SLOT(updateBusy(size_t)), Qt::DirectConnection);
            QObject::connect(this, SIGNAL(cancel()), s.get(), SLOT(cancel()), Qt::DirectConnection);
        }
    }

    ~DirectoryIndexer() {
        for (auto t : threads) {
            t->quit();
            t->wait();
        }
    }

    bool isCancelled() {
        return cancelled;
    }

    quint64 getProgress() {
        return progressCounter;
    }

    quint64 getDirSize() {
        return directorySize;
    }

    static void getTrigrams(DirectoryIndexer * indexer,
                            std::map<size_t, QString> * names,
                            std::unordered_map<uint32_t, std::unordered_set<size_t>> * trigrams) {
        indexer->thread->start();

        for (auto t : indexer->threads) {
            t->start();
        }
        indexer->directorySize = indexer->getDirectorySize(indexer->dirName);

        indexer->processDirectory(QDir(indexer->dirName));

        indexer->waitOneBusy();
        //while (indexer->isBusy1 || indexer->isBusy2/*indexer->wasSent != indexer->manager->wasSent*/) { ;}

        while (indexer->wasSent > indexer->manager->wasSent) {;}

        std::swap(indexer->manager->blocks, *trigrams);
        std::swap(indexer->fileNames, *names);

        indexer->thread->quit();
        indexer->thread->wait();

        for (auto t : indexer->threads) {
            t->quit();
            t->wait();
        }
    }

signals:
    void cancel();

    void sendError(Message m);

public slots:

    void cancelSearching() {
        cancelled = true;
        for (auto s : searchers) {
            s->cancel();
        }
        emit cancel();
    }

    void updateBusy(size_t index) {
        m.lock();
        if (index == 0) {
            state = (state == BusyThreads::BOTH) ? BusyThreads::SECOND : BusyThreads::NONE;
            //isBusy1 = false;
        } else if (index == 1) {
            state = (state == BusyThreads::BOTH) ? BusyThreads::FIRST : BusyThreads::NONE;
            //isBusy2 = false;
        }
        m.unlock();
        workers.notify_all();
    }


private:

    void waitAllBusy() {
        std::unique_lock<std::mutex> lock(m);
        workers.wait(lock, [&] () { return state != BusyThreads::BOTH; });
    }

    void waitOneBusy() {
        std::unique_lock<std::mutex> lock(m);
        workers.wait(lock, [&] () { return state == BusyThreads::NONE; });
    }

    quint64 getDirectorySize(QDir directory) {
        quint64 size = 0;

        if (cancelled) return 0;

        QFileInfoList dirList = directory.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::NoSymLinks);

        for (auto d : dirList) {
            if (d.isFile()) {
                size++;
            } else if (d.isDir()) {
                size += getDirectorySize(d.absoluteFilePath());
            }
        }

       return size;
    }

    void processDirectory(QDir dir) {
        QFileInfoList dirList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::NoSymLinks);

        for (auto d : dirList) {
            if (isCancelled())
                return;

            if (d.isFile())
                progressCounter++;

            if (!d.isReadable()) {
                emit sendError({d.fileName(), Message::WARNING, "File is not readable"});
                continue;
            }

            if (d.isFile() && db.mimeTypeForFile(d.absoluteFilePath()).name().startsWith("text")) {
                insertFile(d.filePath(), ++counter);
            } else if (d.isDir()) {
                processDirectory(d.filePath());
            }
        }
    }

    void insertFile(QString const& fileName, size_t fileIndex) {
        fileNames.insert({fileIndex, fileName.right(fileName.size() - dirName.size())});

        waitAllBusy();

        m.lock();
        if (state == BusyThreads::NONE || state == BusyThreads::SECOND) {
            state = (state == BusyThreads::NONE)? BusyThreads::FIRST : BusyThreads::BOTH;//isBusy1 = true;
            emit s1(fileName, fileIndex);
        } else if (BusyThreads::BOTH != state) {
            state = BusyThreads::BOTH;//(state == BusyThreads::NONE)? BusyThreads::FIRST : BusyThreads::BOTH;//isBusy1 = true;
            //isBusy2 = true;
            emit s2(fileName, fileIndex);
        }
        m.unlock();
        //allWorkers.notify_all();
        wasSent++;

    }


signals:
    void s1(QString fileName, size_t fileIndex);

    void s2(QString fileName, size_t fileIndex);

    void getFreeWorker();

private:
    enum class BusyThreads {
        NONE, FIRST, SECOND, BOTH
    };

    BusyThreads state = BusyThreads::NONE;

    std::condition_variable workers;

    std::mutex m;

    QMimeDatabase db;

    size_t threadsNumber = 2;

    //std::atomic<bool> isBusy1 = {false};

    //std::atomic<bool> isBusy2 = {false};


    size_t wasSent = 0;

    quint64 progressCounter = 0;

    std::atomic<quint64> directorySize = {0};

    size_t counter = 0;


    QString dirName;

    QThread * thread;

    std::vector<std::shared_ptr<QThread>> threads;

    std::vector<std::shared_ptr<TrigramSearcher>> searchers;


    std::unique_ptr<SetManager> manager;

    std::map<size_t, QString> fileNames;

    std::atomic<bool> cancelled = {false};

};

#endif // DIRECTORYINDEXER_H
