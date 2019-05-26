//
// Created by rene on 25.05.19.
//

#ifndef BUNQUEUE_H
#define BUNQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

class BunQueue {
    std::condition_variable cond;
    std::mutex mutex;
    std::queue<char> queue;
    int maxBuns;

public:
    BunQueue(int maxSize) : maxBuns(maxSize) {}

    void addBuns(int amount) {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock, [this](){
            return queue.size() <= maxBuns;
        });
        for (int i = 0; i<amount; i++) queue.push('1');
        lock.unlock();
        cond.notify_all();
    }

    void consumeBuns(int bunNumber, int* receivePtr) {
        int bunAmount = bunNumber;
        std::unique_lock<std::mutex> lock(mutex);
        printf("Waiting for %i buns.\n", bunAmount);
        cond.wait(lock, [this](){
            return queue.size() != 0;
        });
        if (bunAmount > queue.size()) bunAmount = queue.size();
        printf("%i buns available.\n", bunAmount);
        for (int i = 0; i < bunAmount; i++) queue.pop();
        *receivePtr = bunAmount;
        lock.unlock();
        cond.notify_all();
    }

    int length() const {
        return queue.size();
    }
};

#endif //BUNQUEUE_H
