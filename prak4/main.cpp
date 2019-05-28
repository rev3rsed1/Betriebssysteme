#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <sys/time.h>
#include <unistd.h>
#include "BunQueue.h"
#include "ClientQueue.h"

int maxQueueSize = 5;
int maxBuns = 1024;
int clientNumber = 20;
int sellerNumber = 5;
int clientsFinished = 0;

std::condition_variable finished[5]; //size = clientnumber

void produces(BunQueue& bq, ClientQueue& cq) {
    while(clientsFinished < clientNumber) {
        bq.addBuns(15);
        usleep(50);
    }

    printf("Baker finished.\n");
    cq.clear();
}

void sells(BunQueue& bq, ClientQueue& cq, int sellerId) {

    while(clientsFinished < clientNumber) {
        int clientId, bunNumber;
        int* receivePtr;
        //std::condition_variable* cndPtr;

        bunNumber = cq.serveClient(sellerId, &clientId, &receivePtr);

        if (bunNumber > 0) {
            printf("Received order of %i buns.\n", bunNumber);
            bq.consumeBuns(bunNumber, receivePtr);
            finished[clientId].notify_all();
            printf("Completed order with %i buns.\n", *receivePtr);
        }

        pthread_yield();
    }

    printf("Seller ID: %i finished.\n", sellerId);
}

void buys(ClientQueue& cq, int clientId) {
    int bunNumber = 25;
    int receivedBuns = 0;
    int* receivePtr = &receivedBuns;

    std::mutex mtx;

    while (cq.length() >= maxQueueSize) usleep(500);

    printf("Client %i trying to buy %i buns.\n", clientId, bunNumber);

    std::unique_lock<std::mutex> lock(mtx);
    cq.requestServe(bunNumber, clientId, receivePtr);
    finished[clientId].wait(lock, [receivePtr](){
        return *receivePtr != 0;
    });

    clientsFinished++;
    lock.unlock();


    printf("Client %i got %i buns. Finished: %i \n", clientId, receivedBuns, clientsFinished);

}

int main() {
    // cli param n=mitarbeiter,

    BunQueue buns(maxBuns);
    ClientQueue clientQueue(maxQueueSize);



    std::thread clients[clientNumber];
    std::thread sellers[sellerNumber];

    std::thread baker(produces, std::ref(buns), std::ref(clientQueue));
    for (int i = 0; i < clientNumber; i++) {
        clients[i] = std::thread(buys, std::ref(clientQueue), i);
    }
    for (int i = 0; i < sellerNumber; i++) {
        sellers[i] = std::thread(sells, std::ref(buns), std::ref(clientQueue),  i);
    }


    for (int i = 0; i < clientNumber; i++) {
        clients[i].join();
    }
    for (int i = 0; i < sellerNumber; i++) {
        sellers[i].join();
    }
    baker.join();

    return 0;
}

