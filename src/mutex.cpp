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
#include <climits>
#include <alsa/asoundlib.h>
#include <math.h>
#include <chrono>

#define LOOP_SIZE 200
#define GARBAGE_SIZE 1000

typedef std::chrono::high_resolution_clock Clock;
std::chrono::_V2::system_clock::time_point t1;
std::chrono::_V2::system_clock::time_point t2;
clock_t clck;

void initSharedMemory()
{
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 32000000, 0666 | IPC_CREAT);
    unsigned char *str = (unsigned char *)shmat(shmid, (void *)0, 0);
}

void processA(shared_mutex_t mutexA, shared_mutex_t mutexB)
{
    // shared_mutex_t mutex = shared_mutex_init("/my-mutex");
    // if (mutex.ptr == NULL)
    // {
    //     std::cout << "Could not initialise mutex" << std::endl;
    //     return;
    // }

    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 32000000, 0666 | IPC_CREAT);
    unsigned char *str = (unsigned char *)shmat(shmid, (void *)0, 0);

    int rate = 44100;
    const uint16_t freq = 250;
    long unsigned int bufferSize = 4087 * 4;
    const uint16_t len = bufferSize * 16;
    const float_t arg = 2 * 3.141592 * freq / rate;
    long int vals[len + GARBAGE_SIZE];
    int i = 0;
    for (i; i < len; i = i + 1)
    {
        vals[i] = SHRT_MAX * sin(arg * i);
    }

    for(int i = len; i < GARBAGE_SIZE + len; i++)
    {
        vals[i] = 1234567890;
    }

    //std::cout << "AAAA" << std::endl;
    //petla od tąd
    int loop = 0;
    while (loop < LOOP_SIZE)
    {
        pthread_mutex_lock(mutexA.ptr);
        std::chrono::time_point<std::chrono::system_clock> startTime = std::chrono::system_clock::now();
        auto duration = startTime.time_since_epoch();
        auto nano = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        vals[len] = nano;
        // pthread_mutex_lock(mutex.ptr);
        memcpy(str, vals, sizeof(vals));
        //std::cout << "A" << std::endl;
        // pthread_mutex_unlock(mutex.ptr);

        loop++;
        pthread_mutex_unlock(mutexB.ptr);
    }
    //sleep(20);
    shmdt(str);

    //pthread_mutex_unlock(mutex.ptr);
    //shared_mutex_close(mutex);
}

void processB(shared_mutex_t mutexA, shared_mutex_t mutexB)
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
    int i = 0;
    for (i; i < len; i = i + 1)
    {
        vals[i] = 10 * sin(arg * i);
    }

    std::cout << "BBBB" << std::endl;
    // pthread_mutex_lock(mutex.ptr);
    memcpy(&vals, str, sizeof(vals));

     snd_pcm_hw_params_alloca(&hwparams);

     ret = snd_pcm_open(&pcm_handle, pcm_name, stream, 0);
    // std::cout << "Opening: " << snd_strerror(ret) << std::endl;

     ret = snd_pcm_hw_params_any(pcm_handle, hwparams);
    // std::cout << "Initializing hwparams structure: " << snd_strerror(ret) << std::endl;

     ret = snd_pcm_hw_params_set_access(pcm_handle, hwparams,
                                        SND_PCM_ACCESS_RW_INTERLEAVED);
    // std::cout << "Setting access: " << snd_strerror(ret) << std::endl;

     ret = snd_pcm_hw_params_set_format(pcm_handle, hwparams,
                                       SND_PCM_FORMAT_S16_LE);
    // std::cout << "Setting format: " << snd_strerror(ret) << std::endl;

     ret = snd_pcm_hw_params_set_rate(pcm_handle, hwparams,
                                      rate, (int)0);
    // std::cout << "Setting rate: " << snd_strerror(ret) << std::endl;

     ret = snd_pcm_hw_params_set_channels(pcm_handle, hwparams, 2);
    // std::cout << "Setting channels: " << snd_strerror(ret) << std::endl;

     ret = snd_pcm_hw_params_set_periods(pcm_handle, hwparams, 2, 0);
    // std::cout << "Setting periods: " << snd_strerror(ret) << std::endl;

     ret = snd_pcm_hw_params_set_buffer_size_near(pcm_handle, hwparams,
                                                  &bufferSize);
    // std::cout << "Setting buffer size: " << snd_strerror(ret) << std::endl;

     ret = snd_pcm_hw_params(pcm_handle, hwparams);
    // std::cout << "Applying parameters: " << snd_strerror(ret) << std::endl;

    long int valsTmp[len + GARBAGE_SIZE];
    //pętla jakoś od tąd

    int loop = 0;
    std::printf("loop;microseconds;\n");
    while (loop < LOOP_SIZE)
    {
        pthread_mutex_lock(mutexB.ptr);
        //shared memory receive
        memcpy(&valsTmp, str, sizeof(valsTmp));

        clock_t startTime = valsTmp[len];

        i = 0;
        for (i; i < len; i = i + 1)
        {
            vals[i] = valsTmp[i];
        }

        std::chrono::time_point<std::chrono::system_clock> endTimeTmp = std::chrono::system_clock::now();
        auto duration = endTimeTmp.time_since_epoch();
        auto endTime = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

        int err;
        const void *ptra = (const void *)&vals;
        err = snd_pcm_prepare(pcm_handle);
        // std::cout << "Preparing: " << snd_strerror(err) << std::endl;
         while (err != 0)
         {
             err = snd_pcm_prepare(pcm_handle);
         }
         snd_pcm_writei(pcm_handle, ptra, len / 4);

        std::printf("%i;%ld;\n", loop, (endTime - startTime));

        loop++;
        //pthread_mutex_unlock(mutex.ptr);
        pthread_mutex_unlock(mutexA.ptr);
    }

    shared_mutex_close(mutexA);
    shared_mutex_destroy(mutexA);
    shared_mutex_close(mutexB);
    shared_mutex_destroy(mutexB);

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
// compile: g++ mutex.cpp shared_mutex.c -pthread -lstdc++ -pthread -lrt -lm -lasound -o mutex.out
int main()
{
    shared_mutex_t mutexA = shared_mutex_init("/my-mutexA");
    if (mutexA.ptr == NULL)
    {
        std::cout << "Could not initialise mutex" << std::endl;
        //return;
    }
    shared_mutex_t mutexB = shared_mutex_init("/my-mutexB");
    if (mutexB.ptr == NULL)
    {
        std::cout << "Could not initialise mutex" << std::endl;
        //return;
    }

    initSharedMemory();
    std::cout << "main" << std::endl;
    //createProc(processA);
    if (fork() == 0)
    {
        processA(mutexA, mutexB);
        exit(0);
    }
    std::cout << "main" << std::endl;
    sleep(3);
    //createProc(processB);
    if (fork() == 0)
    {
        processB(mutexA, mutexB);
        exit(0);
    }
    std::cout << "main" << std::endl;

    while (wait(NULL) > 0)
    {
    }

    //shared_mutex_close(mutex);
    //shared_mutex_destroy(mutex);

    //shared_mutex_close(mutex);
    //shared_mutex_destroy(mutex);

    return 0;
}