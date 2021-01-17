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

#include <alsa/asoundlib.h>
#include <cmath>
#include <climits>
#include <time.h>
#include <chrono>
typedef std::chrono::high_resolution_clock Clock;
std::chrono::_V2::system_clock::time_point t1;
std::chrono::_V2::system_clock::time_point t2;
clock_t clck;

#define MSG_SIZE 4096

void initSharedMemory()
{
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 64000000, 0666 | IPC_CREAT);
    unsigned char *str = (unsigned char *)shmat(shmid, (void *)0, 0);

   
}

//void processA(mqd_t mqAB, mqd_t mqBA, mq_attr attr)
void processA(mqd_t mqAB, mqd_t mqBA)
{
    char buffer[sizeof(int)];

    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 32000000, 0666 | IPC_CREAT);
    unsigned char *str = (unsigned char *)shmat(shmid, (void *)0, 0);

    
    
    int rate = 44100;
    const uint16_t freq = 250;
    long unsigned int bufferSize = 4087*4;
    const uint16_t len = bufferSize*16;
    const float_t arg = 2 * 3.141592 * freq / rate;
    uint16_t vals[len];
    int i = 0;
    for(i; i < len; i = i + 1) {
        vals[i] = SHRT_MAX * sin(arg*i);
    }


    mqAB = mq_open("/queueAtoB", O_WRONLY);
    mqBA = mq_open("/queueBtoA", O_RDONLY);


    memcpy(str, vals, sizeof(vals));

    memset(buffer, 0, sizeof(int));
    mq_send(mqAB, buffer, sizeof(int), 0);

    bool zakonczono = false;
    int a = 0;
    while (a < 15)
    {
        ssize_t bytes_read;
       // std::cout<<"A"<<std::endl;
        bytes_read = mq_receive(mqBA, buffer, sizeof(int), NULL);
        clck = clock();
        t1 = Clock::now();
        if (bytes_read > 0)
        {
            if (!strncmp(buffer, "111", strlen("111")))
            {
                std::cout << "Odebrana wiadomosc zakonczenia to: " << buffer << std::endl;
                zakonczono = true;
            }

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

    mqBA = mq_open("/queueBtoA", O_WRONLY);
    mqAB = mq_open("/queueAtoB", O_RDONLY);


int ret;

    snd_pcm_t* pcm_handle;  // device handle
    snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
    snd_pcm_hw_params_t* hwparams;  // hardware information
    char* pcm_name = strdup("plughw:0,0");  // on-board audio jack
    int rate = 44100;

    const uint16_t freq = 240;
    long unsigned int bufferSize = 4087*4;
    const uint16_t len = bufferSize*16;
    const float_t arg = 2 * 3.141592 * freq / rate;
    uint16_t vals[len];
    int i = 0;
    for(i; i < len; i = i + 1) {
        vals[i] = 10 * sin(arg*i);
    }

    

    snd_pcm_hw_params_alloca(&hwparams);

    ret = snd_pcm_open(&pcm_handle, pcm_name, stream, 0);
    //std::cout << "Opening: " << snd_strerror(ret) << std::endl;

    ret = snd_pcm_hw_params_any(pcm_handle, hwparams);
    //std::cout << "Initializing hwparams structure: " << snd_strerror(ret) << std::endl;   

    ret = snd_pcm_hw_params_set_access(pcm_handle, hwparams,
            SND_PCM_ACCESS_RW_INTERLEAVED);
    //std::cout << "Setting access: " << snd_strerror(ret) << std::endl;

    ret = snd_pcm_hw_params_set_format(pcm_handle, hwparams,
            SND_PCM_FORMAT_S16_LE);
    //std::cout << "Setting format: " << snd_strerror(ret) << std::endl;

    ret = snd_pcm_hw_params_set_rate(pcm_handle, hwparams,
            rate, (int)0);
    //std::cout << "Setting rate: " << snd_strerror(ret) << std::endl;

    ret = snd_pcm_hw_params_set_channels(pcm_handle, hwparams, 2); 
    //std::cout << "Setting channels: " << snd_strerror(ret) << std::endl;

    ret = snd_pcm_hw_params_set_periods(pcm_handle, hwparams, 2, 0);
    //std::cout << "Setting periods: " << snd_strerror(ret) << std::endl;

    ret = snd_pcm_hw_params_set_buffer_size_near(pcm_handle, hwparams,
            &bufferSize);
    //std::cout << "Setting buffer size: " << snd_strerror(ret) << std::endl;

    ret = snd_pcm_hw_params(pcm_handle, hwparams);
    //std::cout << "Applying parameters: " << snd_strerror(ret) << std::endl;

    bool zakonczono = false;
    int a = 0;
    while (a < 15)
    {
        ssize_t bytes_read;

        bytes_read = mq_receive(mqAB, buffer, sizeof(int), NULL);
       // std::cout<<"B"<<std::endl;
        if (bytes_read > 0)
        {
            if (!strncmp(buffer, "111", strlen("111")))
            {
                std::cout << "Odebrana wiadomosc zakonczenia to: " << buffer << std::endl;
                zakonczono = true;
            }

            //shared memory receive
            memcpy(&vals, str,sizeof(vals));
            clck = clock() - clck;
            t2 = Clock::now();
            int err;
            const void* ptra = (const void*)&vals;
            err = snd_pcm_prepare(pcm_handle);
            //std::cout << "Preparing: " << snd_strerror(err)
            //    << std::endl;
            while(err!=0)
            {   
                err = snd_pcm_prepare(pcm_handle); 
                //std::cout<<"AA"; 
            }
            snd_pcm_writei(pcm_handle, ptra, len);
            std::printf("time measured: %d clicks, %f seconds\n",clck,((float)clck)/CLOCKS_PER_SEC);
            std::cout<<"time measured by better clock: "<<
            std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count()<<std::endl;
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
    // g++ src/MsgQue.cpp -pthread -o MsgQue  -lstdc++ -pthread -lrt -lasound
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
    mqBA = mq_open("/queueBtoA", O_CREAT, 0644, &attr);
    mqAB = mq_open("/queueAtoB", O_CREAT, 0644, &attr);
    //creating new process
    if (fork() == 0)
    {
        //mqAB = mq_open("/queueAtoB", O_WRONLY);
        //mqBA = mq_open("/queueBtoA", O_CREAT | O_RDONLY, 0644, &attr);

        processA(mqAB, mqBA);
        exit(0);
    }

    //std::cout << "main" << std::endl;
    //sleep(3);
    //createProc(processB);

    //creating new process
    if (fork() == 0)
    {
        //mqBA = mq_open("/queueBtoA", O_WRONLY);
        //mqAB = mq_open("/queueAtoB", O_CREAT | O_RDONLY, 0644, &attr);

        processB(mqAB, mqBA);
        exit(0);
    }

    //std::cout << "main" << std::endl;

    while (wait(NULL) > 0)
    {
        //std::cout << "a";
    }
    
    mq_unlink("/queueBtoA");
    mq_unlink("/queueAtoB");   
    return 0;
}