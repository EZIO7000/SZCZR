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



void initSharedMemory()
{
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 32000000, 0666 | IPC_CREAT);
    unsigned char *str = (unsigned char *)shmat(shmid, (void *)0, 0);

}

void processA(pthread_spinlock_t lock, int ret)
{

    ret = pthread_spin_lock(&lock);

    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 32000000, 0666 | IPC_CREAT);
    unsigned char *str = (unsigned char *)shmat(shmid, (void *)0, 0);
    
    int rate = 44100;
    const uint16_t freq = 440;
    long unsigned int bufferSize = 4087*4;
    const uint16_t len = bufferSize*16;
    const float_t arg = 2 * 3.141592 * freq / rate;
    uint16_t vals[len];
    int i = 0;
    for(i; i < len; i = i + 1) {
        vals[i] = SHRT_MAX * sin(arg*i);
    }

    std::cout<<"A"<<std::endl;
    memcpy(str, vals, sizeof(vals));
    std::cout<<"A"<<std::endl;

    shmdt(str);

    ret = pthread_spin_unlock(&lock);
}

void processB(pthread_spinlock_t lock, int ret1)
{
    ret1 = pthread_spin_lock(&lock);

    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key, 32000000, 0666 | IPC_CREAT);
    unsigned char *str = (unsigned char *)shmat(shmid, (void *)0, 0);
    
    
    int ret;

    snd_pcm_t* pcm_handle;  // device handle
    snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
    snd_pcm_hw_params_t* hwparams;  // hardware information
    char* pcm_name = strdup("plughw:0,0");  // on-board audio jack
    int rate = 44100;

    const uint16_t freq = 440;
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
    std::cout << "Opening: " << snd_strerror(ret) << std::endl;

    ret = snd_pcm_hw_params_any(pcm_handle, hwparams);
    std::cout << "Initializing hwparams structure: " << snd_strerror(ret) << std::endl;   

    ret = snd_pcm_hw_params_set_access(pcm_handle, hwparams,
            SND_PCM_ACCESS_RW_INTERLEAVED);
    std::cout << "Setting access: " << snd_strerror(ret) << std::endl;

    ret = snd_pcm_hw_params_set_format(pcm_handle, hwparams,
            SND_PCM_FORMAT_S16_LE);
    std::cout << "Setting format: " << snd_strerror(ret) << std::endl;

    ret = snd_pcm_hw_params_set_rate(pcm_handle, hwparams,
            rate, (int)0);
    std::cout << "Setting rate: " << snd_strerror(ret) << std::endl;

    ret = snd_pcm_hw_params_set_channels(pcm_handle, hwparams, 2); 
    std::cout << "Setting channels: " << snd_strerror(ret) << std::endl;

    ret = snd_pcm_hw_params_set_periods(pcm_handle, hwparams, 2, 0);
    std::cout << "Setting periods: " << snd_strerror(ret) << std::endl;

    ret = snd_pcm_hw_params_set_buffer_size_near(pcm_handle, hwparams,
            &bufferSize);
    std::cout << "Setting buffer size: " << snd_strerror(ret) << std::endl;

    ret = snd_pcm_hw_params(pcm_handle, hwparams);
    std::cout << "Applying parameters: " << snd_strerror(ret) << std::endl;

 
    
    int newValues[4];

    int isEmpty = 1;
    int newRows = 0, newCols = 0, newType = 0;
    
    
    std::cout<<"B"<<std::endl;
        memcpy(&vals, str, sizeof(vals));
        std::cout<<"B"<<std::endl;
        

    int err;
    const void* ptra = (const void*)&vals;
    err = snd_pcm_prepare(pcm_handle);
    std::cout << "Preparing: " << snd_strerror(err)<< std::endl;
    while(err!=0)
        {   
            err = snd_pcm_prepare(pcm_handle);  
        }
    snd_pcm_writei(pcm_handle, ptra, len);
    

    ret1 = pthread_spin_unlock(&lock);
    
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