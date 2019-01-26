#ifndef PAIRSEARCHER_H
#define PAIRSEARCHER_H

#include <QObject>
#include <cstdlib>
#include <set>
#include <QFile>
#include <iostream>
#include <QApplication>

#include "sendsetevent.h"


struct TrigramSearcher : QObject {
    Q_OBJECT
public:
    TrigramSearcher(size_t index) ;
    TrigramSearcher(size_t index,
                 std::vector<size_t> numbers,
                 std::vector<QString> * names) ;

    TrigramSearcher(TrigramSearcher const& searcher) ;

public slots:

    void cancel() {
        cancelled = true;
    }

    void insertFileSlot(QString fileName, size_t fileIndex) ;

    size_t getCode(char c) ;

    uint32_t insertFromBuffer(QByteArray const& buffer, size_t l, size_t r, size_t fileIndex) ;

signals:
    void fileSet(size_t fileIndex, std::set<uint32_t> substrings);

    void finishedSet(size_t searcherIndex);

private:
    size_t searcherIndex;

    std::vector<size_t> numbers;

    std::vector<QString> * names;

    static const size_t CHARSET = 256;

    const size_t BUFFER_SIZE = 2000000;

    std::atomic<bool> cancelled = {false};
};

#endif // PAIRSEARCHER_H
