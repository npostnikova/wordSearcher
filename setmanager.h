#ifndef SETMANAGER_H
#define SETMANAGER_H

#include <QObject>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <cstdlib>
#include <iostream>
#include <atomic>

struct SetManager : QObject {
    Q_OBJECT
public:

    std::atomic<quint64> wasSent = {0};
    std::atomic<bool> cancelled = {false};

public slots:
    void cancel() {
        cancelled = true;
    }

    void updateSet(size_t fileIndex, std::set<uint32_t> values) {
        try {
            for (auto v : values)
                insertValue(v, fileIndex);
            wasSent++;
        } catch (...) {
            wasSent++;
            throw;
        }

    }

    void insertValue(uint32_t value, size_t fileIndex) {
        auto block = blocks.find(value);
        if (block == blocks.end()) {
            blocks.insert({value, {fileIndex}});
        } else {
            block->second.insert(fileIndex);

        }
    }
public:
    std::unordered_map<uint32_t, std::unordered_set<size_t>> blocks;
};

#endif // SETMANAGER_H
