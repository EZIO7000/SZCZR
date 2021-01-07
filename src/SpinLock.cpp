//#include "SpinLock.hpp"

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

void initSharedMemory()
{
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 32000000, 0666 | IPC_CREAT);
    unsigned char *str = (unsigned char *)shmat(shmid, (void *)0, 0);

    int sentValues[4];
    sentValues[0] = 1; //isEmpty = true
    sentValues[1] = 0;
    sentValues[2] = 0;
    sentValues[3] = 0;
    memcpy(str, sentValues, 4 * sizeof(int));
}

void processA(pthread_spinlock_t lock, int ret)
{

    ret = pthread_spin_lock(&lock);

    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 32000000, 0666 | IPC_CREAT);
    unsigned char *str = (unsigned char *)shmat(shmid, (void *)0, 0);
    
    int sentValues[5];
    sentValues[0] = 9; //isEmpty = false - flaga do ozanczenia, ze juz wstawilismy klatke
    sentValues[1] = 9;
    sentValues[2] = 9;
    sentValues[3] = 9;
    sentValues[4] = 4;

    std::cout<<"A"<<std::endl;
    memcpy(str, sentValues, 5 * sizeof(int));
    std::cout<<"A"<<std::endl;

    shmdt(str);

    ret = pthread_spin_unlock(&lock);
}

void processB(pthread_spinlock_t lock, int ret)
{
    ret = pthread_spin_lock(&lock);

    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 32000000, 0666 | IPC_CREAT);
    unsigned char *str = (unsigned char *)shmat(shmid, (void *)0, 0);
    int newValues[4];

    int isEmpty = 1;
    int newRows = 0, newCols = 0, newType = 0;
    
    
    std::cout<<"B"<<std::endl;
        memcpy(&newValues, str, 4 * sizeof(int));
        std::cout<<"B"<<std::endl;
        std::cout<<"New Vals:"<<newValues[0]<<" New Vals:"<<newValues[1]<<" New Vals:"<<newValues[2]<<std::endl;

    shmdt(str);
    shmctl(shmid, IPC_RMID, NULL);

    ret = pthread_spin_unlock(&lock);
}

void createProc(void (*function)())
{
    if (fork() == 0)
    {
        function();
        exit(0);
    }
}

int main()
{
    // INSTRUKCJA DO ODPALENIA 
    // g++ src/SpinLock.cpp -pthread -o SpinLock  -lstdc++ -pthread -lrt
    // ./SpinLock

    pthread_spinlock_t lock;
    int pshared;
    int ret;
    ret = pthread_spin_init(&lock, pshared);

    initSharedMemory();

    std::cout<<"main"<<std::endl;
    //createProc(processA);

    //creating new process
    if (fork() == 0)
    {
        processA(lock, ret);
        exit(0);
    }

    std::cout<<"main"<<std::endl;
    sleep(3);
    //createProc(processB);

    //creating new process
    if (fork() == 0)
    {
        processB(lock, ret);
        exit(0);
    }

    std::cout<<"main"<<std::endl;

    while (wait(NULL) > 0)
    {
        
    }
    
    ret = pthread_spin_destroy(&lock);

    return 0;
}