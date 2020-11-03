//
// Created by rrzhang on 2020/11/1.
// 和 send-revice.cpp 区别在于测试 waitUntilCompleted 的用法。
//


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>

#include <infinity/core/Context.h>
#include <infinity/queues/QueuePairFactory.h>
#include <infinity/queues/QueuePair.h>
#include <infinity/memory/Buffer.h>
#include <infinity/memory/RegionToken.h>
#include <infinity/requests/RequestToken.h>

#define PORT_NUMBER 8011
#define SERVER_IP "127.0.0.1"

// Usage: ./progam -s for server and ./program for client component
int main(int argc, char **argv) {

    bool isServer = false;

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

    infinity::core::Context *context = new infinity::core::Context();
    infinity::queues::QueuePairFactory *qpFactory = new  infinity::queues::QueuePairFactory(context);
    infinity::queues::QueuePair *qp;

    if(isServer) {
        printf("Creating buffers to receive a messages\n");
        infinity::memory::Buffer *sendBuffer = new infinity::memory::Buffer(context, sizeof(uint64_t));
        infinity::memory::Buffer *receiveBuffer = new infinity::memory::Buffer(context, sizeof(uint64_t));
        context->postReceiveBuffer(receiveBuffer);


        printf("Waiting for incoming connection\n");
        qpFactory->bindToPort(PORT_NUMBER);
        qp = qpFactory->acceptIncomingConnection();
        std::cout << "connected." << std::endl;

        infinity::core::receive_element_t receiveElement;
        uint64_t temp;
        while(1) {
            while (!context->receive(&receiveElement));
            memcpy(&temp, receiveBuffer->getData(), sizeof(uint64_t));
            std::cout << "recive: " << temp << std::endl;
            context->postReceiveBuffer(receiveElement.buffer);


            uint64_t temp2 = temp + 1;
            memcpy(sendBuffer->getData(), &temp2, sizeof(uint64_t));
            qp->send(sendBuffer, context->defaultRequestToken);
//            context->defaultRequestToken->waitUntilCompleted();

            std::cout << "return: " << temp2 << std::endl;


            if(temp == 10 - 1) break;
        }

        delete sendBuffer;
        delete receiveBuffer;

    } else {
        printf("Connecting to remote node\n");
        qp = qpFactory->connectToRemoteHost(SERVER_IP, PORT_NUMBER);
        printf("Creating buffers\n");
        infinity::memory::Buffer *sendBuffer = new infinity::memory::Buffer(context, sizeof(uint64_t) * 10);
        infinity::memory::Buffer **reciveBuffer = (infinity::memory::Buffer **)calloc(10, sizeof(infinity::memory::Buffer *));

        for (uint64_t i = 0; i < 10; i++) {
            reciveBuffer[i] = new infinity::memory::Buffer(context, sizeof(uint64_t));
            context->postReceiveBuffer(reciveBuffer[i]);
        }


        infinity::core::receive_element_t receiveElement;
        char *ptr = (char*)sendBuffer->getData();
        uint64_t i;
        for( i = 0; i < 9; i++) {
            memcpy(ptr, &i, sizeof(uint64_t));
            qp->send(sendBuffer, i*sizeof(uint64_t), sizeof(uint64_t), infinity::queues::OperationFlags());
            ptr += sizeof(uint64_t);
        }
        assert(i == 9);
        memcpy(ptr, &i, sizeof(uint64_t));
        qp->send(sendBuffer, 9*sizeof(uint64_t), sizeof(uint64_t), infinity::queues::OperationFlags(), context->defaultRequestToken);
        context->defaultRequestToken->waitUntilCompleted();

        for (uint64_t i = 0; i < 10; i++) {
            uint64_t temp;
            while (!context->receive(&receiveElement));
            memcpy(&temp, receiveElement.buffer->getData(), sizeof(uint64_t));
            std::cout << "recive: " << temp << std::endl;
//            assert(temp == i + 1);
            context->postReceiveBuffer(receiveElement.buffer);
        }


        delete sendBuffer;
        delete reciveBuffer;
    }


    delete qp;
    delete qpFactory;
    delete context;
    return 0;
}
