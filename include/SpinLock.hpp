#pragma once

#include <time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>

#include <stdlib.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>

class SpinLock {
    pthread_spinlock_t lock;
    int pshared;
    int ret;

    void initSharedMemory();
    void processA();
    void processB();
    void createProc(void (*function)());
    int main();


public:
    SpinLock();
    ~SpinLock();
};