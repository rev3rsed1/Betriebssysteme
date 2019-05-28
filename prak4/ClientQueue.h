//
// Created by rene on 25.05.19.
//

#ifndef CLIENTQUEUE_H
#define CLIENTQUEUE_H
#include <queue>
#include <mutex>
#include <condition_variable>
#include "BunQueue.h"

class ClientQueue {
    struct Client{
        int clientId;
        int* receivePtr;
        int bunNumber;
        //std::condition_variable* waiting;
    };

    std::condition_variable cond;
    std::mutex mutex;
    std::queue<Client> queue;
    int maxClients;
    bool finished;

public:
    ClientQueue(int maxSize) : maxClients(maxSize), finished(false) {}

    void requestServe(int bunAmount, int clientId, int* receivedBuns) {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock, [this](){
            return queue.size() <= maxClients;
        });
        struct Client cl = {clientId, receivedBuns, bunAmount};
        queue.push(cl);
        lock.unlock();
        cond.notify_all();
    }

    int serveClient(int sellerId, int* clientId, int** receivePtr) {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock, [this](){
            return queue.size() != 0 || finished;
        });
        int receive = 0;
        if(!finished) {
            printf("Seller %i serving Client %i ...\n", sellerId, queue.front().clientId);
            *receivePtr = queue.front().receivePtr;
            *clientId = queue.front().clientId;
            receive = queue.front().bunNumber;
            queue.pop();
        }
        lock.unlock();
        cond.notify_all();
        return receive;
    }

    int length() const {
        return queue.size();
    }

    void clear() {
        std::unique_lock<std::mutex> lock(mutex);
        while(queue.size() > 0) queue.pop();
        finished = true;
        lock.unlock();
        cond.notify_all();
    }
};

#endif //CLIENTQUEUE_H
