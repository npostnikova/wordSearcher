#include "wordsearcher.h"
#include <QFuture>
#include <QtConcurrent/QtConcurrentRun>
#include <QApplication>
#include <QTimer>
#include "hashsearcher.h"

WordSearcher::WordSearcher() : worker(new SetWorker), thread(new QThread)
{
    QObject::connect(this, SIGNAL(joinSet(std::unordered_set<size_t>, bool)), worker.get(), SLOT(joinSets(std::unordered_set<size_t>, bool)));
    QObject::connect(this, SIGNAL(unionSet(std::unordered_set<size_t>, bool)), worker.get(), SLOT(intersectSets(std::unordered_set<size_t>, bool)));


    worker->moveToThread(thread.get());

}

WordSearcher::~WordSearcher() {}


std::vector<QString> WordSearcher::runSearcher(QString const& word, QString const& dir, std::unordered_map<uint32_t, std::unordered_set<size_t>> * blocks,
                                               std::map<size_t, QString> * fileNames) {
    try {
        cancelled = false;
        m.lock();
        successfullyCancelled = false;
        m.unlock();

        this->word = word;
        this->dir = dir;
        this->fileNames = fileNames;
        this->blocks = blocks;

        worker->clean();
        thread->start();

        findWord(word);

        auto filesWithTrigrams = worker->getIndexes();

        thread->quit();
        thread->wait();

        auto res = checkContaining(filesWithTrigrams, word);

        if (!cancelled) emit finishedSearching();

        m.lock();
        successfullyCancelled = true;
        m.unlock();
        cv.notify_all();
        return res;
    } catch(...) {
        emit error({"Memory allocation problems", Message::ERROR, "Please, choose smaller directory"});
    }
    return {};

}

std::vector<QString> WordSearcher::checkContaining(std::unordered_set<size_t> const& indexes, QString const& word) {
    HashSearcher * hashSearcher = new HashSearcher;
    std::vector<QString> result;
    for (auto i : indexes) {
        if (cancelled) break;
        if (hashSearcher->contain(word.toStdString(), dir + fileNames->find(i)->second)) {// QDir(dir).filePath(fileNames->find(i)->second))) {
            result.push_back(dir + fileNames->find(i)->second);
        }
        if (result.size() >= 10) {
            emit sendFiles(result);
            result.clear();
        }
    }
    if (!cancelled && result.size() > 0) {
        emit sendFiles(result);
    }
    return result;
}


void WordSearcher::findWord(QString const& word) {
    if (static_cast<size_t>(word.length()) >= BLOCK_SIZE) {
        findGoodString(word.toStdString());
    } else {
        findBadString(word.toStdString());
    }
    worker->waitForDone();
}

void WordSearcher::findGoodString(std::string const& word) {
    auto wordTrigrams = getWordTrigrams(word);
    auto it = wordTrigrams.begin();

    while (it != wordTrigrams.end()) {
        auto values = getFilesWithTrigram(*it);
        if (values.empty() || cancelled) {
            emit unionSet({}, true);
            if (cancelled) break;
            return;
        } else {
            emit unionSet(values, ++it == wordTrigrams.end());
        }
    }
}


uint32_t WordSearcher::createTrigram(std::string const& str) {
    uint32_t trigram = 0;
    for (auto s : str) {
        trigram <<= 8;
        trigram += getCode(s);
    }
    return trigram;
}

std::set<uint32_t> WordSearcher::getWordTrigrams(std::string const& str) {
    std::set<uint32_t> result;

    for (size_t i = 0; i < str.size() - BLOCK_SIZE + 1 && !cancelled; i++) {
        result.insert(createTrigram(str.substr(i, BLOCK_SIZE)));
    }
    return result;
}


void WordSearcher::findRec(uint32_t trigram, std::string const& word) {
    for (size_t i = 0; i < 256; i++) {
        if (word.length() == 2) {
            bool b = cancelled.load();
            emit joinSet(getFilesWithTrigram((trigram << 8) + i), i == 255 || b);
            if (b) break;
        } else {
            bool b = cancelled.load();
            for (size_t j = 0; j < 256; j++) {
                emit joinSet(getFilesWithTrigram((((trigram << 8) + i) << 8) + j), i == 255 && j == 255 || b);
                if (b) goto exit;
            }
        }
    }
    exit:
    ;
}

void WordSearcher::findBadString(std::string const& word) {
    findRec(word.length() == 1 ? getCode(word[0]) : ((getCode(word[0]) << 8) + getCode(word[1])), word);
}


std::unordered_set<size_t> WordSearcher::getFilesWithTrigram(uint32_t trigram) {
    auto block = blocks->find(trigram);
    if (block == blocks->end())
        return {};
    else
        return block->second;

}
