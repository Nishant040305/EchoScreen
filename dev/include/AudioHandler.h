#ifndef AUDIOHANDLER_H
#define AUDIOHANDLER_H

#include <Arduino.h>
#include <driver/i2s.h>
#include <WebSocketsClient.h>

// I2S Configuration
#define I2S_PORT I2S_NUM_0
#define I2S_WS 15
#define I2S_SCK 14
#define I2S_SD 32

#define SAMPLE_RATE 16000
#define BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_32BIT
#define I2S_READ_LEN 1024 // Number of samples to read

class AudioHandler
{
private:
    bool isRecording;
    bool isInitialized;
    unsigned long lastSendTime;
    uint32_t chunkCount;

public:
    AudioHandler();
    bool init();
    void startRecording();
    void stopRecording();
    void processAudioChunk();
    void cleanup();
    bool getIsRecording() { return isRecording; }
};

extern AudioHandler audioHandler;

#endif