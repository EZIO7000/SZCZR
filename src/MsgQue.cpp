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

#define LOOP_SIZE 200
#define GARBAGE_SIZE 1000

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
    long int vals[len + GARBAGE_SIZE];
    for(uint16_t i = 0; i < len; i = i + 1) {
        vals[i] = SHRT_MAX * sin(arg*i);
    }

    for(int i = len; i < GARBAGE_SIZE + len; i++)
    {
        vals[i] = 1234567890;
    }

    mqAB = mq_open("/queueAtoB", O_WRONLY);
    mqBA = mq_open("/queueBtoA", O_RDONLY);

    std::chrono::time_point<std::chrono::system_clock> startTime = std::chrono::system_clock::now();
    auto duration = startTime.time_since_epoch();
    auto nano = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    vals[len] = nano;
    memcpy(str, vals, sizeof(vals));

    memset(buffer, 0, sizeof(int));
    mq_send(mqAB, buffer, sizeof(int), 0);

    bool zakonczono = false;
    int a = 0;
    while (a < LOOP_SIZE)
    {
        ssize_t bytes_read;
        bytes_read = mq_receive(mqBA, buffer, sizeof(int), NULL);
        t1 = Clock::now();
        if (bytes_read > 0)
        {
            if (!strncmp(buffer, "111", strlen("111")))
            {
                std::cout << "Odebrana wiadomosc zakonczenia to: " << buffer << std::endl;
                zakonczono = true;
            }

            startTime = std::chrono::system_clock::now();
            auto duration = startTime.time_since_epoch();
            auto nano = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
            vals[len] = nano;
            memcpy(str, vals, sizeof(vals));


            memset(buffer, 0, sizeof(int));
            mq_send(mqAB, buffer, sizeof(int), 0);

            a++;
        }
    }

    shmdt(str);
    shmctl(shmid, IPC_RMID, NULL);
}

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
    for(uint16_t i = 0; i < len; i = i + 1) {
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
    std::printf("loop;microseconds;\n");
    while (a < LOOP_SIZE)
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

            long int valsTmp[len + GARBAGE_SIZE];
            memcpy(&valsTmp, str, sizeof(valsTmp));

            clock_t startTime = valsTmp[len];

            for(int i = 0; i < len; i = i + 1) {
                vals[i] = valsTmp[i];
            }

            std::chrono::time_point<std::chrono::system_clock> endTimeTmp = std::chrono::system_clock::now();
            auto duration = endTimeTmp.time_since_epoch();
            auto endTime = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
            
            t2 = Clock::now();
             int err;
             const void* ptra = (const void*)&vals;
            err = snd_pcm_prepare(pcm_handle);
            while(err!=0)
            {   
                err = snd_pcm_prepare(pcm_handle); 
            }
            snd_pcm_writei(pcm_handle, ptra, len/4);

        
            std::printf("loop nr %i ;%ld;\n",a,(endTime - startTime));

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
    mq_unlink("/queueBtoA");
    mq_unlink("/queueAtoB");

    initSharedMemory();

    mqd_t mqAB;              // message queue
    mqd_t mqBA;
    mq_attr attr; // message attributes

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(int);
    attr.mq_curmsgs = 0;

    clock_t startTime = 0;
    clock_t endTime;

    mqBA = mq_open("/queueBtoA", O_CREAT, 0644, &attr);
    mqAB = mq_open("/queueAtoB", O_CREAT, 0644, &attr);
    if (fork() == 0)
    {
        processA(mqAB, mqBA);
        exit(0);
    }

    if (fork() == 0)
    {
        processB(mqAB, mqBA);
        exit(0);
    }

    while (wait(NULL) > 0)
    {
    }
    
    mq_unlink("/queueBtoA");
    mq_unlink("/queueAtoB");   
    return 0;
}