#include <pthread.h>
#include <sys/sysinfo.h>
#include <vector>
#include <iostream>
#include <chrono>
#include "test.h"


#define CPP11  
#ifdef CPP11_ATOMIC
    #include <atomic>
    #undef CPP11
#endif

struct ProduserArgs{
    std::shared_ptr<MCMP<int>> queue;
    int startNum;
    int endNum;
};

struct ConsumerArgs{
    std::shared_ptr<MCMP<int>> queue;
    #ifdef CPP11_ATOMIC
        std::vector<std::atomic<int>>& nums;
    #else
        std::vector<int>& nums;
    #endif
    int numberForOneTest;
};

static void* Produser(void* args){
    ProduserArgs* params = (ProduserArgs*)args;
    for (int i = params->startNum; i <= params->endNum; ++i)
        params->queue->enqueue(i);
    return nullptr;
}

static void* Consumer(void* args){
    ConsumerArgs* params = (ConsumerArgs*)args;
    int readNum = 0;
    while(readNum < params->numberForOneTest){
        std::shared_ptr<int> num = params->queue->dequeue();
        if (num != nullptr){
            #ifdef CPP11_ATOMIC
                params->nums[*num]++;
            #else
            #ifdef CPP11
                __atomic_add_fetch(&params->nums[*num], 1, __ATOMIC_RELAXED);
            #else
                __sync_fetch_and_add(&params->nums[*num], 1);
            #endif
            #endif
            readNum++;
        }   
    }
    return nullptr;
}

bool pushTest(size_t threadNum, int number, int repeatNum){
    auto queue = std::make_shared<MCMP<int>>();
    std::vector<ProduserArgs*> produsersArgs;
    std::vector<pthread_t> thProdusers;

    thProdusers.resize(threadNum * repeatNum);
    produsersArgs.resize(threadNum * repeatNum);

    auto start = std::chrono::steady_clock::now();
    for (int num = 0; num < repeatNum; ++num){
        for (size_t i = 0; i < threadNum; ++i){
            produsersArgs[num * threadNum + i] = new ProduserArgs{queue, (int)(i * number), (int)((i + 1) * number - 1)};
            int res = pthread_create(&thProdusers[num * threadNum + i], NULL, Produser, (void*)produsersArgs[num * threadNum + i]);
            if (res != 0){
                logMessage("ERROR: thread create failed. Number: " + std::to_string(num * threadNum + i), LOG_ERR);
            }
        }
        for (size_t i = 0; i < threadNum; ++i){
            int res = pthread_join(thProdusers[num * threadNum + i], nullptr);
            if (res != 0){
                logMessage("ERROR: thread join failed " + std::to_string(num * threadNum + i), LOG_ERR);
            }
        }
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    logMessage("Test time: " + std::to_string(elapsed_seconds.count()), LOG_INFO);
    logMessage("Mean test time: " + std::to_string(elapsed_seconds.count() / repeatNum), LOG_INFO);

    for (size_t i = 0; i < threadNum; ++i){
        delete produsersArgs[i];
    }

    std::vector<int> numInQueue;
    numInQueue.resize(threadNum * number);
    for (size_t i = 0; i < threadNum * number * repeatNum; ++i){
        int num;
        if (queue->empty()){
            logMessage("FAILED TEST: not enougth number in queue " + std::to_string(i), LOG_ERR);
            return false;
        }
        num = *queue->dequeue();
        numInQueue[num]++;
    }

    if (!queue->empty()){
        logMessage("FAILED TEST: to many number in queue", LOG_ERR);
        return false;
    }
    for (size_t i = 0; i < threadNum * number; ++i)
        if(numInQueue[i] != repeatNum){
            logMessage("FAILED TEST: to many repeat " + std::to_string(i) + " in queue", LOG_ERR);
            return false;
        }

    return true;
}

bool popTest(size_t threadNum, int number, int repeatNum){
    auto queue = std::make_shared<MCMP<int>>();
    std::vector<ConsumerArgs*> consumersArgs;
    std::vector<pthread_t> thConsumers;
    #ifdef CPP11_ATOMIC
        std::vector<std::atomic<int>> checkingVec(number * threadNum * repeatNum);
    #else
        std::vector<int> checkingVec(number * threadNum * repeatNum);
    #endif
    thConsumers.resize(threadNum * repeatNum);
    consumersArgs.resize(threadNum * repeatNum);

    for (size_t i = 0; i < number * threadNum * repeatNum; ++i){
        checkingVec[i] = 0;
    }

    for (size_t i = 0; i < number * threadNum * repeatNum; ++i){
        queue->enqueue(i);
    }

    auto start = std::chrono::steady_clock::now();
    for (int num = 0; num < repeatNum; ++num){
        for (size_t i = 0; i < threadNum; ++i){
            consumersArgs[num * threadNum + i] = new ConsumerArgs{queue, checkingVec, number};
            int res = pthread_create(&thConsumers[num * threadNum + i], NULL, Consumer, (void*)consumersArgs[num * threadNum + i]);
            if (res != 0){
                logMessage("ERROR: thread create failed. Number: " + std::to_string(num * threadNum + i), LOG_ERR);
            }
        }
        for (size_t i = 0; i < threadNum; ++i){
            int res = pthread_join(thConsumers[num * threadNum + i], nullptr);
            if (res != 0){
                logMessage("ERROR: thread join failed " + std::to_string(num * threadNum + i), LOG_ERR);
            }
        }
    }
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    logMessage("Test time: " + std::to_string(elapsed_seconds.count()), LOG_INFO);
    logMessage("Mean test time: " + std::to_string(elapsed_seconds.count() / repeatNum), LOG_INFO);

    for (size_t i = 0; i < threadNum * repeatNum; ++i){
        delete consumersArgs[i];
    }
    

    if (!queue->empty()){
        logMessage("FAILED TEST: to many number in queue", LOG_ERR);
        return false;
    }
    for (size_t i = 0; i < number * threadNum; ++i)
        #ifdef CPP11_ATOMIC
            if(checkingVec[i].load() != 1){
                logMessage("FAILED TEST: to many repeat " + std::to_string(i) + " in queue", LOG_ERR);
                return false;
            }
        #else
            if(checkingVec[i] != 1){
                logMessage("FAILED TEST: to many repeat " + std::to_string(i) + " in queue", LOG_ERR);
                return false;
            }
        #endif

    return true;
}

bool pushPopTest(int number, int repeatNum){
    int maxThreadNum = get_nprocs_conf();
    std::cout << maxThreadNum << std::endl;
    int produserThreadNum;
    int consumerThreadNum;
   for (produserThreadNum = 1; produserThreadNum < maxThreadNum; ++produserThreadNum){
        for (consumerThreadNum = 1; consumerThreadNum + produserThreadNum <= maxThreadNum; ++consumerThreadNum){
            std::cout << produserThreadNum << ' ' << consumerThreadNum << std::endl;

            auto queue = std::make_shared<MCMP<int>>();
            std::vector<ProduserArgs*> produsersArgs(produserThreadNum * repeatNum);
            std::vector<pthread_t> thProdusers(produserThreadNum * repeatNum);
            std::vector<ConsumerArgs*> consumersArgs(consumerThreadNum * repeatNum);
            std::vector<pthread_t> thConsumers(consumerThreadNum * repeatNum);
            #ifdef CPP11_ATOMIC
                std::vector<std::atomic<int>> checkingVec(number * threadNum * repeatNum);
            #else
                std::vector<int> checkingVec(number);
            #endif

            for (int i = 0; i < number; ++i){
                checkingVec[i] = 0;
            }

            auto start = std::chrono::steady_clock::now();
            for (int num = 0; num < repeatNum; ++num){
                for (int i = 0; i < produserThreadNum; ++i){
                    int startNum = (int)(i * (number / produserThreadNum));
                    int endNum;
                    if (i == produserThreadNum - 1)
                        endNum = number - 1;
                    else 
                        endNum = (int)((i + 1) * (number / produserThreadNum) - 1);
                    produsersArgs[num * produserThreadNum + i] = new ProduserArgs{queue, startNum, endNum};
                    int res = pthread_create(&thProdusers[num * produserThreadNum + i], NULL, Produser, (void*)produsersArgs[num * produserThreadNum + i]);
                    if (res != 0){
                        logMessage("ERROR: thread create failed. Number: " + std::to_string(num * produserThreadNum + i), LOG_ERR);
                    }
                }
                for (int i = 0; i < consumerThreadNum; ++i){
                    int number;
                    if (i == produserThreadNum - 1)
                        number = number - number / consumerThreadNum * (consumerThreadNum - 1);
                    else
                        number = number / consumerThreadNum;
                    consumersArgs[num * consumerThreadNum + i] = new ConsumerArgs{queue, checkingVec, number};
                    int res = pthread_create(&thConsumers[num * consumerThreadNum + i], NULL, Consumer, (void*)consumersArgs[num * consumerThreadNum + i]);
                    if (res != 0){
                        logMessage("ERROR: thread create failed. Number: " + std::to_string(num * produserThreadNum + i), LOG_ERR);
                    }
                }

                for (int i = 0; i < produserThreadNum; ++i){
                    int res = pthread_join(thProdusers[num * produserThreadNum + i], nullptr);
                if (res != 0){
                    logMessage("ERROR: thread join failed " + std::to_string(num * produserThreadNum + i), LOG_ERR);
                }
                }
                for (int i = 0; i < consumerThreadNum; ++i){
                    int res = pthread_join(thConsumers[num * consumerThreadNum + i], nullptr);
                    if (res != 0){
                        logMessage("ERROR: thread join failed " + std::to_string(num * produserThreadNum + i), LOG_ERR);
                    }
                }
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            logMessage("Test time: " + std::to_string(elapsed_seconds.count()), LOG_INFO);
            logMessage("Mean test time: " + std::to_string(elapsed_seconds.count() / repeatNum), LOG_INFO);

            for (int i = 0; i < produserThreadNum; ++i){
                delete produsersArgs[i];
            }
            for (int i = 0; i < consumerThreadNum * repeatNum; ++i){
                delete consumersArgs[i];
            }

            if (!queue->empty()){
                logMessage("FAILED TEST: to many number in queue", LOG_ERR);
                return false;
            }
            for (int i = 0; i < number; ++i)
            #ifdef CPP11_ATOMIC
                if(checkingVec[i].load() != repeatNum){
                    logMessage("FAILED TEST: to many repeat " + std::to_string(i) + " in queue", LOG_ERR);
                    return false;
                }
            #else
                if(checkingVec[i] != repeatNum){
                    logMessage("FAILED TEST: to many repeat " + std::to_string(i) + " in queue", LOG_ERR);
                    return false;
                }
        #endif
        }
    }
    return true;
}

void logMessage(const std::string& message, int priority = LOG_INFO) {
    std::cout << message << std::endl;  
    if (priority == LOG_ERR || priority == LOG_NOTICE){
        syslog(priority, "%s", message.c_str());
    }
}

void runPushTest(const std::vector<std::smatch>& matches) {
    int threadNum = atoi(matches[0][1].str().c_str());
    int number = atoi(matches[0][2].str().c_str());
    int repeatNum = atoi(matches[0][3].str().c_str());

    logMessage("Push test:");
    logMessage("Threads: " + std::to_string(threadNum) + ", Elements: " + std::to_string(number) +
               ", Repeat: " + std::to_string(repeatNum));
    
    bool testRes = pushTest(std::size_t(threadNum), number, repeatNum);

    if (testRes == true){
        logMessage("Success Push test", LOG_NOTICE);
    } else {
        logMessage("Failed Push test", LOG_ERR);
    }
}

void runPopTest(const std::vector<std::smatch>& matches) {
    int threadNum = atoi(matches[0][1].str().c_str());
    int number = atoi(matches[0][2].str().c_str());
    int repeatNum = atoi(matches[0][3].str().c_str());
    
    logMessage("Pop test:");
    logMessage("Threads: " + std::to_string(threadNum) + ", Elements: " + std::to_string(number) +
               ", Repeat: " + std::to_string(repeatNum));
    
    bool testRes = popTest(std::size_t(threadNum), number, repeatNum);
    
    if (testRes == true){
        logMessage("Success Pop test", LOG_NOTICE);
    } else {
        logMessage("Failed Pop test", LOG_ERR);
    }
}

void runPushPopTest(const std::vector<std::smatch>& matches) {
    int number = atoi(matches[0][1].str().c_str());
    int repeatNum = atoi(matches[0][2].str().c_str());
    
    logMessage("Push Pop test:");
    logMessage("Number: " + std::to_string(number) + ", Repeat: " + std::to_string(repeatNum));
    
    bool testRes = pushPopTest(number, repeatNum);
    
    if (testRes == true){
        logMessage("Success Push Pop test", LOG_NOTICE);
    } else {
        logMessage("Failed Push Pop test", LOG_ERR);
    }
}