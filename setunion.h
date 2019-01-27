#ifndef SETUNION_H
#define SETUNION_H

#include <QString>
#include <unordered_set>
#include <cstdlib>
#include <QObject>

#include <mutex>
#include <condition_variable>

#include <iostream>

struct SetWorker : QObject {
    Q_OBJECT
public:
    SetWorker(size_t unionIndex = 0) : unionIndex(unionIndex) {}

    ~SetWorker() {}

    void waitForDone() {
        std::unique_lock<std::mutex> lock(m);
        checkDone.wait(lock, [&] () { return done;} );
    }

public slots:
    void joinSets(std::unordered_set<size_t> s, bool last = false) {
        if (isCancelled())
            return;
        for (auto file : s) {
            if (isCancelled())
                break;
            setUnion.insert(file);
        }
        if (last) {
            m.lock();
            done = true;
            m.unlock();
            checkDone.notify_all();
        }
    }

    void intersectSets(std::unordered_set<size_t> s, bool last = false) {

        if (isCancelled())
            return;
        if (!wasInitialized()) {
            initialized = true;
            std::swap(s, setUnion);
            if (last) {
                m.lock();
                done = true;
                m.unlock();
                checkDone.notify_all();
            }

            return;
        }
        std::unordered_set<size_t> result;
        for (auto file : s) {
            if (isCancelled())
                break;
            if (setUnion.count(file) != 0) {
                result.insert(file);
            }
        }
        std::swap(result, setUnion);
        if (last) {
            m.lock();
            done = true;
            m.unlock();
            checkDone.notify_all();
        }
    }

    bool isDone() {
        return done;
    }

    std::unordered_set<size_t> getIndexes() {
        return setUnion;
    }

    size_t getIndex() {
        return unionIndex;
    }

    void clean() {
        setUnion.clear();
        cancelled = false;

        m.lock();
        done = false;
        m.unlock();
        checkDone.notify_all();
        initialized = false;
    }

private slots:
    void cancel() {
        cancelled = true;
    }

private:
    bool isCancelled() {
        return cancelled;
    }

    bool wasInitialized() {
        return initialized;
    }

    size_t unionIndex;

    std::unordered_set<size_t> setUnion;

    bool cancelled = false;

    bool done = false;

    std::condition_variable checkDone;

    std::mutex m;

    bool initialized = false;
};

#endif // SETUNION_H
