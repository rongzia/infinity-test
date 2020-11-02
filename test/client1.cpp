//
// Created by rrzhang on 2020/11/1.
//

#include <cassert>
#include <chrono>
#include <iostream>

#include <infinity/core/Context.h>
#include <infinity/queues/QueuePairFactory.h>
#include <infinity/queues/QueuePair.h>
#include <infinity/memory/Buffer.h>
#include <infinity/memory/RegionToken.h>
#include <infinity/requests/RequestToken.h>

#define PORT_NUMBER 8011
#define SERVER_IP "127.0.0.1"

int main() {
    infinity::core::Context *context = new infinity::core::Context();
    infinity::queues::QueuePairFactory *qpFactory = new  infinity::queues::QueuePairFactory(context);
    infinity::queues::QueuePair *qp;
    {

        printf("Connecting to remote node\n");
        qp = qpFactory->connectToRemoteHost(SERVER_IP, PORT_NUMBER);
        printf("Creating buffers\n");
        infinity::memory::Buffer *sendBuffer = new infinity::memory::Buffer(context, sizeof(uint64_t));
        infinity::memory::Buffer *reciveBuffer = new infinity::memory::Buffer(context, sizeof(uint64_t));
        context->postReceiveBuffer(reciveBuffer);


        infinity::core::receive_element_t receiveElement;
        for(uint64_t i = 0; i < 10; i++) {
            memcpy(sendBuffer->getData(), &i, sizeof(uint64_t));
            qp->send(sendBuffer, context->defaultRequestToken);
            context->defaultRequestToken->waitUntilCompleted();
            std::cout << "send: " << i << std::endl;

            uint64_t temp;
            while (!context->receive(&receiveElement));
            memcpy(&temp, reciveBuffer->getData(), sizeof(uint64_t));
            std::cout << "recive: " << temp << std::endl;
            assert(temp == i + 1);
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