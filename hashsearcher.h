#ifndef HASHSEARCHER_H
#define HASHSEARCHER_H

#include "filehighlighterdialog.h"
#include "ui_filehighlighterdialog.h"
#include "mylistview.h"

#include <QObject>
#include <QString>
#include <cstdlib>
#include <string>
#include <QFile>
#include <QTextStream>
#include <QTextCharFormat>
#include <iostream>

#include <QSyntaxHighlighter>

struct HashSearcher : QObject {
    Q_OBJECT

public:
    bool contain(std::string const& word, QString const& fileName) {
        QFile file;
        file.setFileName(fileName);
        if (!file.open(QFile::ReadOnly)) {
            emit error({fileName, Message::ERROR, "File cannot be opened"});

        }
        QByteArray buffer;
        while (!file.atEnd()) {
            buffer = file.read(BUFFER_SIZE);
            if (findInBuffer(&buffer, getHash(word), word)) {
                return true;
            }
        }
        return false;
    }

signals:
    void error(Message const& m);

private:

    bool wordsEqual(size_t i, std::string const& word, QByteArray * byteArray) {
        for (size_t j = i - word.length() + 1; j <= i; j++) {
            if (word[j - i + word.length() - 1] != byteArray->at(j)) {
                return false;
            }
        }
        return true;
    }

    bool findInBuffer(QByteArray * byteArray, uint64_t wordHash, std::string const & word) {
        if (byteArray->size() < word.length())
            return false;
        uint64_t curHash = 0;
        uint64_t curX = 1;
        for (size_t i = 0; i < word.length(); i++) {
            if (i != 0) {
                curX = (curX * X) % P;
            }
            curHash = (curHash * X + getCode(byteArray->at(i))) % P;
        }

        if (curHash == wordHash && word == byteArray->left(word.length()).toStdString()) {
            return true;
        }

        for (size_t i = word.length(); i < byteArray->size(); i++) {
            curHash = (curHash + P - (getCode(byteArray->at(i - word.length())) * curX) % P) % P;

            curHash = (curHash * X + getCode(byteArray->at(i))) % P;

            if (curHash == wordHash && wordsEqual(i, word, byteArray)) {
                    return true;
            }
        }
        return false;
    }

    static uint64_t getHash(const std::string& string) {
        long long result = 0;
        for (size_t i = 0; i < string.size(); i++) {
            result = (result * X + getCode(string[i])) % P;
            }
        return result % P;
    }


    static size_t getCode(char c) {
        return static_cast<size_t>(static_cast<int>(CHARSET) + static_cast<int>(c)) % CHARSET;
    }


    static const size_t CHARSET = 256;

    static const size_t X = 263;

    static const size_t P = 1000000007;

    static const size_t BUFFER_SIZE = 2000000;
};

#endif // HASHSEARCHER_H
