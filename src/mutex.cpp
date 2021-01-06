#include <unistd.h>
#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>

#include <stdlib.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>

#include <time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include "shared_mutex.h"

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

void processA()
{
    shared_mutex_t mutex = shared_mutex_init("/my-mutex");
    if (mutex.ptr == NULL)
    {
        std::cout << "Could not initialise mutex" << std::endl;
        return;
    }
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
   // pthread_mutex_lock(mutex.ptr);
    std::cout<<"A"<<std::endl;
    memcpy(str, sentValues, 5 * sizeof(int));
   // pthread_mutex_unlock(mutex.ptr);
    

    sleep(20);
    shmdt(str);
    //shared_mutex_close(mutex);
}

void processB()
{
    shared_mutex_t mutex = shared_mutex_init("/my-mutex");
    if (mutex.ptr == NULL)
    {
        std::cout << "Could not initialise mutex" << std::endl;
        return;
    }
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 32000000, 0666 | IPC_CREAT);
    unsigned char *str = (unsigned char *)shmat(shmid, (void *)0, 0);
    int newValues[4];
    bool windowCreated = false;
    int isEmpty = 1;
    int newRows = 0, newCols = 0, newType = 0;
    char *tmpData = (char *)malloc(32000000);

   // pthread_mutex_lock(mutex.ptr);
    memcpy(&newValues, str, 4 * sizeof(int));
   // pthread_mutex_unlock(mutex.ptr);
    isEmpty = newValues[0];
    newRows = newValues[1];
    newCols = newValues[2];
    newType = newValues[3];
    
   // cv::Mat newframe;
    int k;
    
    
    std::cout<<"B"<<std::endl;
       // pthread_mutex_lock(mutex.ptr);
        memcpy(&newValues, str, 4 * sizeof(int));
        std::cout<<"B"<<std::endl;
        memcpy(tmpData, str + 4 * sizeof(int), 3 * sizeof(unsigned char) * newRows * newCols);
       // pthread_mutex_unlock(mutex.ptr);
        isEmpty = newValues[0];
        newRows = newValues[1];
        newCols = newValues[2];
        newType = newValues[3];
        std::cout<<"New Vals:"<<newValues[0]<<"New Vals:"<<newValues[1]<<"New Vals:"<<newValues[2]<<std::endl;
        //k=cv::waitKey(1);
        
    shmdt(str);
    shmctl(shmid, IPC_RMID, NULL);
    //shared_mutex_close(mutex);
    //shared_mutex_destroy(mutex);
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
    initSharedMemory();
    std::cout<<"C"<<std::endl;
    createProc(processA);
    std::cout<<"C"<<std::endl;
    sleep(3);
    createProc(processB);
    std::cout<<"C"<<std::endl;

    while (wait(NULL) > 0)
    {
        
    }
    
    return 0;
}