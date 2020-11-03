//
// Created by rrzhang on 2020/11/1.
// 和 send-revice.cpp 区别在于 server 可以给每个链接分配一个后台线程，处理对应链接的操作。
// 每个线程拥有一个 context, 和一个 qp。 QueuePairFactory 是公用的，和原来库的用法不一样。
//


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>
#include "unique_ptr.h"

#include <infinity/core/Context.h>
#include <infinity/queues/QueuePairFactory.h>
#include <infinity/queues/QueuePair.h>
#include <infinity/memory/Buffer.h>
#include <infinity/memory/RegionToken.h>
#include <infinity/requests/RequestToken.h>

#define PORT_NUMBER 8011
#define SERVER_IP "127.0.0.1"

#define ECHO_COUNT (1000000)

void server_thread(std::unique_ptr<infinity::core::Context> context, std::unique_ptr<infinity::queues::QueuePair> qp){

    printf("Creating buffers to receive a messages\n");
    infinity::memory::Buffer *sendBuffer = new infinity::memory::Buffer(context.get(), 16384);
    infinity::memory::Buffer *receiveBuffer = new infinity::memory::Buffer(context.get(), 16384);
    context->postReceiveBuffer(receiveBuffer);

    infinity::core::receive_element_t receiveElement;
    uint64_t i = 0;
    while(1) {
        while (!context->receive(&receiveElement));
        context->postReceiveBuffer(receiveElement.buffer);

        memset(sendBuffer->getData(), 0, 16384);
        qp->send(sendBuffer, context->defaultRequestToken);
        context->defaultRequestToken->waitUntilCompleted();

        i++;
        if(ECHO_COUNT == i) break;
    }

    delete sendBuffer;
    delete receiveBuffer;

//    delete qp;
//    delete context;
    return;
}

// Usage: ./progam -s for server and ./program for client component
int main(int argc, char **argv) {

    bool isServer = false;
    bool shutdowm  = false;

    while (argc > 1) {
        if (argv[1][0] == '-') {
            switch (argv[1][1]) {

                case 's': {
                    isServer = true;
                    break;
                }

            }
        }
        ++argv;
        --argc;
    }




    if(isServer) {
        infinity::queues::QueuePairFactory *qpFactory = new infinity::queues::QueuePairFactory();
        qpFactory->bindToPort(PORT_NUMBER);
        while(!shutdowm) {
            printf("Waiting for incoming connection\n");
            std::unique_ptr<infinity::core::Context> context = std::make_unique<infinity::core::Context>();
            std::unique_ptr<infinity::queues::QueuePair> qp;
            qp.reset(qpFactory->acceptIncomingConnection(context.get()));
            std::cout << "connected." << std::endl;
            std::thread server_th(server_thread, std::move(context), std::move(qp));
            server_th.detach();
        }
        delete qpFactory;
    } else {
        infinity::core::Context *context = new infinity::core::Context();
        infinity::queues::QueuePairFactory *qpFactory = new infinity::queues::QueuePairFactory(context);
        infinity::queues::QueuePair *qp;
        printf("Connecting to remote node\n");
        qp = qpFactory->connectToRemoteHost(SERVER_IP, PORT_NUMBER);
        printf("Creating buffers\n");
        infinity::memory::Buffer *sendBuffer = new infinity::memory::Buffer(context, 16384);
        infinity::memory::Buffer *reciveBuffer = new infinity::memory::Buffer(context, 16384);
        context->postReceiveBuffer(reciveBuffer);


        std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> start = std::chrono::system_clock::now();
        infinity::core::receive_element_t receiveElement;
        for (uint64_t i = 0; i < ECHO_COUNT; i++) {
            memset(sendBuffer->getData(), 0, 16384);
            qp->send(sendBuffer, context->defaultRequestToken);
            context->defaultRequestToken->waitUntilCompleted();
            if(i % (ECHO_COUNT / 10) == 0) { std::cout << "send: " << i << std::endl; }

            while (!context->receive(&receiveElement));
//            if(i % (ECHO_COUNT / 10) == 0) { std::cout << "recive: " << temp << std::endl; }
            context->postReceiveBuffer(receiveElement.buffer);
        }
        std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> end = std::chrono::system_clock::now();
        uint64_t dura = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        std::cout << "average: " << dura / ECHO_COUNT / 1000 << " us." << std::endl;


        delete sendBuffer;
        delete reciveBuffer;


        delete qp;
        delete qpFactory;
        delete context;
    }


    return 0;
}
