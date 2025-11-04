#ifndef AUDIO_HANDLER_H
#define AUDIO_HANDLER_H
#include <constants.h>
#include <Arduino.h>
#include <driver/i2s.h>
#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 16000
#define I2S_READ_LEN 1024
#define BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_32BIT

class AudioHandler {
private:
    bool isRecording;
    bool isInitialized;
    
public:
    AudioHandler();
    bool init();
    void startRecording();
    void stopRecording();
    bool getIsRecording() { return isRecording; }
    void processAudioChunk();
    void cleanup();
};

extern AudioHandler audioHandler;

#endif // AUDIO_HANDLER_H