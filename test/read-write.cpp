//
// Created by rrzhang on 2020/11/1.
// RDMA read-write 完全不需要对方CPU的参与，client 拿到 server 内存的 token 后，就可以自由读写 server 的内存，server 感知不到。
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

        printf("Creating buffers to read from and write to\n");
        infinity::memory::Buffer *bufferToReadWrite = new infinity::memory::Buffer(context, sizeof(uint64_t));
        infinity::memory::RegionToken *bufferToken = bufferToReadWrite->createRegionToken();

        uint64_t a = 10;
        memcpy(bufferToReadWrite->getData(), &a, sizeof(uint64_t));

        printf("Setting up connection (blocking)\n");
        qpFactory->bindToPort(PORT_NUMBER);
        qp = qpFactory->acceptIncomingConnection(bufferToken, sizeof(infinity::memory::RegionToken));

        delete bufferToReadWrite;

    } else {

        printf("Connecting to remote node\n");
        qp = qpFactory->connectToRemoteHost(SERVER_IP, PORT_NUMBER);
        infinity::memory::RegionToken *remoteBufferToken = (infinity::memory::RegionToken *) qp->getUserData();


        printf("Creating buffers\n");
        infinity::memory::Buffer *buffer1Sided = new infinity::memory::Buffer(context, sizeof(uint64_t));

        printf("Reading content from remote buffer\n");
        infinity::requests::RequestToken requestToken(context);
        qp->read(buffer1Sided, remoteBufferToken, &requestToken);
        requestToken.waitUntilCompleted();

        uint64_t a;
        memcpy(&a, buffer1Sided->getData(), sizeof(uint64_t));
        std::cout << a << std::endl;

        a++;
        memcpy(buffer1Sided->getData(), &a, sizeof(uint64_t));

        printf("Writing content to remote buffer\n");
        qp->write(buffer1Sided, remoteBufferToken, &requestToken);
        requestToken.waitUntilCompleted();


        delete buffer1Sided;
    }

    delete qp;
    delete qpFactory;
    delete context;

    return 0;

}
