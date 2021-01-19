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

#include <math.h>
#include <climits>
#include <alsa/asoundlib.h>

#include "shared_spinlock.h"

#include <chrono>

#define LOOP_SIZE 200
#define GARBAGE_SIZE 1000

bool wasSent = false;
void initSharedMemory()
{
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 32000000, 0666 | IPC_CREAT);
    unsigned char *str = (unsigned char *)shmat(shmid, (void *)0, 0);
}

void processA(shared_mutex_t lockA, shared_mutex_t lockB, int ret)
{

    //pthread_spin_lock(lockA.ptr);
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 32000000, 0666 | IPC_CREAT);
    unsigned char *str = (unsigned char *)shmat(shmid, (void *)0, 0);

    int rate = 44100;
    const uint16_t freq = 250;
    long unsigned int bufferSize = 4087 * 4;
    const uint16_t len = bufferSize * 16;
    const float_t arg = 2 * 3.141592 * freq / rate;
    long int vals[len + GARBAGE_SIZE];
    for (uint16_t i = 0; i < len; i = i + 1)
    {
        vals[i] = SHRT_MAX * sin(arg * i);
    }

    for(int i = len; i < GARBAGE_SIZE + len; i++)
    {
        vals[i] = 1234567890;
    }

    // loop from here I guess idk
    int loop = 0;
    while (loop < LOOP_SIZE)
    {
        ret = pthread_spin_lock(lockA.ptr);
    //    memcpy( &vals,str, sizeof(vals));
    //    vals[len] = wasSent;
        
       // std::cout << "A" << std::endl;
        // if(wasSent)
        // {
        //     while (wasSent)
        //     {
        //         //printf("\nProc A: wait for the spinlock...");
        //         ret = pthread_spin_unlock(&lockA);
        //         //sleep(3); 
        //         ret = pthread_spin_lock(&lockA);
        //         memcpy( &vals,str, sizeof(vals));
        //         wasSent = vals[len];
        //         //std::cout<<std::endl<<"loop nr in a"<<a<<"  "<<wasSent;
        //         //ret = pthread_spin_lock(&lockA);
        //     }   
        // }
        std::chrono::time_point<std::chrono::system_clock> startTime = std::chrono::system_clock::now();
        auto duration = startTime.time_since_epoch();
        auto nano = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        vals[len+1] = nano;

        wasSent = true;
        vals[len] = wasSent;
        memcpy(str, vals, sizeof(vals));  
        //std::cout<<std::endl<< "A sent" <<wasSent<< std::endl;
        loop++;
        ret = pthread_spin_unlock(lockB.ptr);
    }

    shmdt(str); // <- to poza petla powinno byc nie? po unlock'u
}

void processB(shared_mutex_t lockA, shared_mutex_t lockB, int ret1)
{
    

    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 32000000, 0666 | IPC_CREAT);
    unsigned char *str = (unsigned char *)shmat(shmid, (void *)0, 0);

    int ret;

    snd_pcm_t *pcm_handle; // device handle
    snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
    snd_pcm_hw_params_t *hwparams;         // hardware information
    char *pcm_name = strdup("plughw:0,0"); // on-board audio jack
    int rate = 44100;

    const uint16_t freq = 240;
    long unsigned int bufferSize = 4087 * 4;
    const uint16_t len = bufferSize * 16;
    const float_t arg = 2 * 3.141592 * freq / rate;
    uint16_t vals[len];
    for (uint16_t i = 0; i < len; i = i + 1)
    {
        vals[i] = 10 * sin(arg * i);
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

    // loop from here
    //std::cout<<pthread_spin_lock(&lockB);
    long int valsTmp[len + GARBAGE_SIZE];
    //memcpy(&valsTmp, str, sizeof(valsTmp));
    //wasSent = valsTmp;
    int loop = 0;
    std::printf("loop;microseconds;\n");
    while (loop < LOOP_SIZE)
    {
        //memcpy(&valsTmp, str, sizeof(valsTmp));
        //wasSent = valsTmp;

        ret = pthread_spin_lock(lockB.ptr);
        // memcpy( &vals,str, sizeof(vals));
        // vals[len] = wasSent;

        // if(!wasSent){
        //     while (!wasSent)
        //     {
        //         ret1 = pthread_spin_unlock(&lockB);
        //         //sleep(3); 
        //         ret1 = pthread_spin_lock(&lockB);
        //         memcpy( &vals,str, sizeof(vals));
        //         wasSent = vals[len];
        //     }
        //     loop++;
        // }
        //pthread_spin_lock(&lockB);
        //ret1 = pthread_spin_lock(&lock);
        //printf("Proc B: made it past while\n");
        //std::cout << "B" << std::endl;
        

        // memcpy(&valsTmp, str, sizeof(valsTmp));
        // wasSent = false;
        valsTmp[len] = wasSent;
        memcpy(str, &valsTmp, sizeof(valsTmp));

        //std::cout << "B sent" <<wasSent<< std::endl;

        

        for (int i = 0; i < len; i = i + 1)
        {
            vals[i] = valsTmp[i];
        }

        clock_t startTime = valsTmp[len+1];
        std::chrono::time_point<std::chrono::system_clock> endTimeTmp = std::chrono::system_clock::now();
        auto duration = endTimeTmp.time_since_epoch();
        auto endTime = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

        int err;
        const void *ptra = (const void *)&vals;
        err = snd_pcm_prepare(pcm_handle);
        //std::cout << "Preparing: " << snd_strerror(err)<< std::endl;
        while (err != 0)
        {
            err = snd_pcm_prepare(pcm_handle);
        }
        snd_pcm_writei(pcm_handle, ptra, len / 4);
        std::printf("%i;%ld;\n", loop, (endTime - startTime));

        loop++;
        //endloop
        ret1 = pthread_spin_unlock(lockA.ptr);
    }

    shared_mutex_close(lockA);
    shared_mutex_destroy(lockA);
    shared_mutex_close(lockB);
    shared_mutex_destroy(lockB);

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
    // g++ src/SpinLock.cpp -pthread -o SpinLock  -lstdc++ -pthread -lrt -lasound
    // g++ SpinLock.cpp shared_spinlock.c -pthread -lstdc++ -pthread -lrt -lm -lasound -o SpinLock
    // ./SpinLock

    shared_mutex_t lockA = shared_mutex_init("/my-lockA");
    if (lockA.ptr == NULL)
    {
        std::cout << "Could not initialise mutex" << std::endl;
        //return;
    }
    shared_mutex_t lockB = shared_mutex_init("/my-lockB");
    if (lockB.ptr == NULL)
    {
        std::cout << "Could not initialise mutex" << std::endl;
        //return;
    }

    //pthread_spinlock_t lockA;
    //pthread_spinlock_t lockB;
    // int pshared;
    int ret;
    // int retB;
    // ret = pthread_spin_init(&lockA, pshared);
    // retB = pthread_spin_init(&lockB, pshared);

    initSharedMemory();

    //std::cout << "main" << std::endl;
    //createProc(processA);

    //creating new process
    if (fork() == 0)
    {
        processA(lockA, lockB , ret);
        exit(0);
    }

    //std::cout << "main" << std::endl;
    sleep(1);
    //createProc(processB);

    //creating new process
    if (fork() == 0)
    {
        processB(lockA, lockB, ret);
        exit(0);
    }

    //std::cout << "main" << std::endl;

    while (wait(NULL) > 0)
    {
    }

    return 0;
}