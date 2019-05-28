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
std::condition_variable finished[200];
int clientsFinished = 0;
int bunRate = 15;
int bakerSleep = 50;



void produces(BunQueue& bq, ClientQueue& cq) {
    while(clientsFinished < clientNumber) {
        bq.addBuns(bunRate);
        usleep(bakerSleep);
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
            printf("Completed order with %i buns.\n", *receivePtr);
            finished[clientId].notify_all();
        }

        pthread_yield();
    }

    printf("Seller ID: %i finished.\n", sellerId);
}

void buys(ClientQueue& cq, int clientId) {
    int bunNumber = (rand() % 30) + 1;
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

int main(int argc, char* argv[]) {
    // cli param b=bunRate, l=queuesize, m=clientnumber, n=sellernumber, r=bakersleep
    if (argc != 6){
        printf("Too few arguments! (b, l, m, n, r)\n");
        return 0;
    }
    maxQueueSize = atoi(argv[2]);
    clientNumber = atoi(argv[3]);
    sellerNumber = atoi(argv[4]);
    bunRate = atoi(argv[1]);
    bakerSleep = atoi(argv[5]);


    BunQueue buns(maxBuns);
    ClientQueue clientQueue(maxQueueSize);


    std::thread clients[clientNumber];
    std::thread sellers[sellerNumber];

    std::thread baker(produces, std::ref(buns), std::ref(clientQueue));
    for (int i = 0; i < clientNumber; i++) {
        clients[i] = std::thread(buys, std::ref(clientQueue), i);
    }
    for (int i = 0; i < sellerNumber; i++) {
        sellers[i] = std::thread(sells, std::ref(buns), std::ref(clientQueue), i);
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

