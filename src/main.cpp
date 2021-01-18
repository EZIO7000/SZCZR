#include <cmath>
#include <climits>
#include <iostream>
#include <alsa/asoundlib.h>


using namespace std;

int main() {
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
    uint16_t vals[len/2];
    int i = 0;
    for(i; i < len; i = i + 1) {
        vals[i] = SHRT_MAX * sin(arg*i);
    }

    

    snd_pcm_hw_params_alloca(&hwparams);

    ret = snd_pcm_open(&pcm_handle, pcm_name, stream, 0);
    cout << "Opening: " << snd_strerror(ret) << endl;

    ret = snd_pcm_hw_params_any(pcm_handle, hwparams);
    cout << "Initializing hwparams structure: " << snd_strerror(ret) << endl;   

    ret = snd_pcm_hw_params_set_access(pcm_handle, hwparams,
            SND_PCM_ACCESS_RW_INTERLEAVED);
    cout << "Setting access: " << snd_strerror(ret) << endl;

    ret = snd_pcm_hw_params_set_format(pcm_handle, hwparams,
            SND_PCM_FORMAT_S16_LE);
    cout << "Setting format: " << snd_strerror(ret) << endl;

    ret = snd_pcm_hw_params_set_rate(pcm_handle, hwparams,
            rate, (int)0);
    cout << "Setting rate: " << snd_strerror(ret) << endl;

    ret = snd_pcm_hw_params_set_channels(pcm_handle, hwparams, 2); 
    cout << "Setting channels: " << snd_strerror(ret) << endl;

    ret = snd_pcm_hw_params_set_periods(pcm_handle, hwparams, 2, 0);
    cout << "Setting periods: " << snd_strerror(ret) << endl;

    ret = snd_pcm_hw_params_set_buffer_size_near(pcm_handle, hwparams,
            &bufferSize);
    cout << "Setting buffer size: " << snd_strerror(ret) << endl;

    ret = snd_pcm_hw_params(pcm_handle, hwparams);
    cout << "Applying parameters: " << snd_strerror(ret) << endl;
    
    int err;
    cout << endl << endl;

    
    const void* ptr = (const void*)&vals;
    ptr+=bufferSize;
for( int z = 0; z<10;z++) {
    ptr = (const void*)&vals;
    do {
        if(ret < 0) {
            err = snd_pcm_prepare(pcm_handle);
            cout << "Preparing: " << snd_strerror(err)
                << endl;
            
        }
        //ptr += bufferSize;
        ret = snd_pcm_writei(pcm_handle, ptr, len/2);
    } while(ret < 0);
}
    cout << "Writing data: " << ret << ", " << snd_strerror(ret)<< endl;
}