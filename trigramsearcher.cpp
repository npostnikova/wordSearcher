#include "trigramsearcher.h"
#include <QMimeDatabase>

 TrigramSearcher::TrigramSearcher(size_t index/*, Trie * trie*/) : searcherIndex(index) {}
    TrigramSearcher::TrigramSearcher(size_t index,
                 std::vector<size_t> numbers,
                 std::vector<QString> * names) : searcherIndex(index), numbers(numbers), names(names){}
   // TrigramSearcher::TrigramSearcher(TrigramSearcher const& searcher) : searcherIndex(searcher.searcherIndex), numbers(searcher.numbers), names(searcher.names) {}


    void TrigramSearcher::insertFileSlot(QString fileName, size_t fileIndex) {
        try {
        QFile file;
        file.setFileName(fileName);
        if (!file.open(QFile::ReadOnly)) {
            emit finishedSet(searcherIndex);
            emit fileSet(fileIndex, {});
            return;
        }


        std::set<uint32_t> result;

        QByteArray buffer;
        QMimeDatabase db;

        while (!file.atEnd()) {
            buffer.append(file.read(BUFFER_SIZE));

            if (!db.mimeTypeForData(buffer).name().startsWith("text")) break;

            for (int i = 0; !cancelled && i < buffer.size(); i++) {
                result.insert(insertFromBuffer(buffer, i, std::min(i + 3, buffer.size()), fileIndex));
            }
                if (buffer.size() < 2) break;
                buffer = buffer.right(2);

            }
            file.close();
            emit fileSet(fileIndex, result);
            emit finishedSet(searcherIndex);
        } catch(...) {
            cancelled = true;
            emit finishedSet(searcherIndex);
            emit fileSet(fileIndex, {});
        }

    }


    size_t TrigramSearcher::getCode(char c) {
        return static_cast<size_t>(static_cast<int>(CHARSET) + static_cast<int>(c)) % CHARSET;
    }


    uint32_t TrigramSearcher::insertFromBuffer(QByteArray const& buffer, size_t l, size_t r, size_t fileIndex) {
        uint32_t triple = 0;
        while (l < r) {
            triple <<= 8;
            size_t c = getCode(buffer.at(static_cast<int>(l)));

            triple += c;
            l++;
        }

        return triple;
    }



