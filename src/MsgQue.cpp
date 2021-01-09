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

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define MSG_SIZE 4096

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

//void processA(mqd_t mqAB, mqd_t mqBA, mq_attr attr)
void processA(mqd_t mqAB, mqd_t mqBA)
{
    char buffer[sizeof(int)];

    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 32000000, 0666 | IPC_CREAT);
    unsigned char *str = (unsigned char *)shmat(shmid, (void *)0, 0);

    int newValues[4];
    newValues[0] = 0; //isEmpty = false - flaga do ozanczenia, ze juz wstawilismy klatke
    newValues[1] = 0;
    newValues[2] = 0;
    newValues[3] = 0;


    memcpy(str, newValues, 4 * sizeof(int));

    memset(buffer, 0, sizeof(int));
    mq_send(mqAB, buffer, sizeof(int), 0);

    bool zakonczono = false;
    int a = 0;
    while (a < 30)
    {
        ssize_t bytes_read;

        bytes_read = mq_receive(mqBA, buffer, sizeof(int), NULL);

        if (bytes_read > 0)
        {
            if (!strncmp(buffer, "111", strlen("111")))
            {
                std::cout << "Odebrana wiadomosc zakonczenia to: " << buffer << std::endl;
                zakonczono = true;
            }

            //shared memory receive
            memcpy(&newValues, str, 4 * sizeof(int));
            std::cout << "Odczytane przez A: New Vals: " << newValues[0] << " New Vals: " << newValues[1] << std::endl;

            //zamiana danych
            newValues[0]++;
            memcpy(str, newValues, 4 * sizeof(int));

            //shared memory send
            memset(buffer, 0, sizeof(int));
            mq_send(mqAB, buffer, sizeof(int), 0);

            a++;
        }
    }

    shmdt(str);
    shmctl(shmid, IPC_RMID, NULL);
}

//void processB(mqd_t mqAB, mqd_t mqBA, mq_attr attr)
void processB(mqd_t mqAB, mqd_t mqBA)
{
    char buffer[sizeof(int) + 1];

    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 32000000, 0666 | IPC_CREAT);
    unsigned char *str = (unsigned char *)shmat(shmid, (void *)0, 0);
    int newValues[4];
    newValues[0] = 0; //isEmpty = false - flaga do ozanczenia, ze juz wstawilismy klatke
    newValues[1] = 0;
    newValues[2] = 0;
    newValues[3] = 0;

    bool zakonczono = false;
    int a = 0;
    while (a < 30)
    {
        ssize_t bytes_read;

        bytes_read = mq_receive(mqAB, buffer, sizeof(int), NULL);

        if (bytes_read > 0)
        {
            if (!strncmp(buffer, "111", strlen("111")))
            {
                std::cout << "Odebrana wiadomosc zakonczenia to: " << buffer << std::endl;
                zakonczono = true;
            }

            //shared memory receive
            memcpy(&newValues, str, 4 * sizeof(int));
            std::cout << "Odczytane przez B: New Vals: " << newValues[0] << " New Vals: " << newValues[1] << std::endl;

            //zamiana danych
            newValues[1]++;
            memcpy(str, newValues, 4 * sizeof(int));

            //shared memory send
            memset(buffer, 0, sizeof(int));
            mq_send(mqBA, buffer, sizeof(int), 0);

            a++;
        }
    }

    shmdt(str);
    shmctl(shmid, IPC_RMID, NULL);
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
    // g++ src/MsgQue.cpp -pthread -o MsgQue  -lstdc++ -pthread -lrt
    // ./MsgQue

    //mqd_t mqdes = mq_open("/msgque", O_RDWR);

    initSharedMemory();

    mqd_t mqAB;              // message queue
    mqd_t mqBA;
    /*struct*/ mq_attr attr; // message attributes

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(int);
    attr.mq_curmsgs = 0;

    //std::cout << "main" << std::endl;
    //createProc(processA);

    //creating new process
    if (fork() == 0)
    {
        mqAB = mq_open("/queueAtoB", O_WRONLY);
        mqBA = mq_open("/queueBtoA", O_CREAT | O_RDONLY, 0644, &attr);

        processA(mqAB, mqBA);
        exit(0);
    }

    //std::cout << "main" << std::endl;
    sleep(3);
    //createProc(processB);

    //creating new process
    if (fork() == 0)
    {
        mqBA = mq_open("/queueBtoA", O_WRONLY);
        mqAB = mq_open("/queueAtoB", O_CREAT | O_RDONLY, 0644, &attr);

        processB(mqAB, mqBA);
        exit(0);
    }

    //std::cout << "main" << std::endl;

    while (wait(NULL) > 0)
    {
        //std::cout << "a";
    }

    return 0;
}