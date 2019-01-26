#ifndef WORDSEARCHER_H
#define WORDSEARCHER_H

#include <cstdlib>
#include <QThread>
#include <unordered_map>
#include "setunion.h"
#include <QString>
#include <QDir>
#include <QObject>
#include <QFileInfoList>
#include "trigramsearcher.h"
#include "mylistview.h"

#include <condition_variable>

struct WordSearcher : QObject {
    Q_OBJECT
public:

    WordSearcher();

    ~WordSearcher();

    void findWord(QString const& word);

    bool wasCancelled() {
        return successfullyCancelled;
    }


public slots:
    std::vector<QString> runSearcher(QString const& word, QString const& dir,
                                     std::unordered_map<uint32_t, std::unordered_set<size_t>> * blocks,
                                     std::map<size_t, QString> * fileNames);
    void cancel() {
        cancelled = true;
    }

signals:
    void unionSet(std::unordered_set<size_t>, bool last);


    void joinSet(std::unordered_set<size_t>, bool last);

    void finishedSearching();

    void error(Message const &m);

    void sendFiles(std::vector<QString> files);
private:

    std::unordered_set<size_t> getFilesWithTrigram(uint32_t trigram);

    size_t getCode(char c) {
        return static_cast<size_t>(static_cast<int>(CHARSET) + static_cast<int>(c)) % CHARSET;
    }

    void findGoodString(std::string const& str);

    void findBadString(std::string const& str);


    std::set<uint32_t> getWordTrigrams(std::string const& str);

    uint32_t createTrigram(std::string const& str);

    void findTrigram(std::string const& word);

    std::vector<QString> checkContaining(std::unordered_set<size_t> const& indexes, QString const& word);

    void findRec(uint32_t, std::string const& word);

    static const size_t BLOCK_SIZE = 3;

    static const size_t CHARSET = 256;


    std::unique_ptr<SetWorker> worker;

    std::unique_ptr<QThread> thread;


    std::unordered_map<uint32_t, std::unordered_set<size_t>> * blocks;

    std::map<size_t, QString> * fileNames;


    QString word;

    QString dir;

    std::atomic<bool> cancelled = {false};

    std::atomic<bool> successfullyCancelled = {false};

};

#endif // WORDSEARCHER_H
