//
// Created by rrzhang on 2020/11/1.
//

#include <cassert>
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
            context->defaultRequestToken->waitUntilCompleted();

            std::cout << "return: " << temp2 << std::endl;


            if(temp == 10 - 1) break;
        }

        delete sendBuffer;
        delete receiveBuffer;

    }
    delete qp;
    delete qpFactory;
    delete context;
    return 0;
}