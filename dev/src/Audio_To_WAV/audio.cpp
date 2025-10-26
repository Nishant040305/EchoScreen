#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstring>
#include "portaudio.h"

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512
#define NUM_CHANNELS 1
#define BITS_PER_SAMPLE 16
#define DURATION_SECONDS 10

typedef int16_t Sample;
#define PA_SAMPLE_TYPE paInt16

void write_chunk_id(std::ofstream &file, const char *id)
{
    file.write(id, 4);
}

void write_int32(std::ofstream &file, uint32_t data)
{
    file.write(reinterpret_cast<const char *>(&data), 4);
}

void write_int16(std::ofstream &file, uint16_t data)
{
    file.write(reinterpret_cast<const char *>(&data), 2);
}

void checkError(PaError err)
{
    if (err != paNoError)
    {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        exit(1);
    }
}

int main()
{
    PaError err;
    PaStream *stream;

    long totalFrames = SAMPLE_RATE * DURATION_SECONDS;
    long totalSamples = totalFrames * NUM_CHANNELS;

    long dataSize = totalSamples * sizeof(Sample);

    std::vector<Sample> audioData(totalSamples);

    std::cout << "Initializing PortAudio..." << std::endl;
    err = Pa_Initialize();
    checkError(err);

    PaStreamParameters inputParameters;

    inputParameters.device = Pa_GetDefaultInputDevice();
    if (inputParameters.device == paNoDevice)
    {
        std::cerr << "Error: No default input device." << std::endl;
        Pa_Terminate();
        return 1;
    }
    inputParameters.channelCount = NUM_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    std::cout << "Opening audio stream..." << std::endl;
    err = Pa_OpenStream(
        &stream,
        &inputParameters,
        NULL,
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,
        NULL,
        NULL);
    checkError(err);

    std::cout << "Starting stream and capturing " << DURATION_SECONDS << " second(s) of audio..." << std::endl;
    err = Pa_StartStream(stream);
    checkError(err);

    err = Pa_ReadStream(stream, audioData.data(), totalFrames);
    checkError(err);

    std::cout << "Finished capture. Stopping stream..." << std::endl;
    err = Pa_StopStream(stream);
    checkError(err);

    err = Pa_CloseStream(stream);
    checkError(err);

    err = Pa_Terminate();
    checkError(err);
    std::cout << "PortAudio terminated successfully." << std::endl;

    const char *filename = "captured_audio.wav";
    std::ofstream outfile(filename, std::ios::binary);

    if (!outfile.is_open())
    {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return 1;
    }

    write_chunk_id(outfile, "RIFF");

    uint32_t riffChunkSize = 36 + dataSize;
    write_int32(outfile, riffChunkSize);
    write_chunk_id(outfile, "WAVE");

    write_chunk_id(outfile, "fmt ");
    uint32_t fmtChunkSize = 16;
    write_int32(outfile, fmtChunkSize);

    uint16_t audioFormat = 1;
    write_int16(outfile, audioFormat);

    uint16_t numChannels = NUM_CHANNELS;
    write_int16(outfile, numChannels);

    uint32_t sampleRate = SAMPLE_RATE;
    write_int32(outfile, sampleRate);

    uint32_t byteRate = SAMPLE_RATE * NUM_CHANNELS * BITS_PER_SAMPLE / 8;
    write_int32(outfile, byteRate);

    uint16_t blockAlign = NUM_CHANNELS * BITS_PER_SAMPLE / 8;
    write_int16(outfile, blockAlign);

    uint16_t bitsPerSample = BITS_PER_SAMPLE;
    write_int16(outfile, bitsPerSample);

    write_chunk_id(outfile, "data");

    write_int32(outfile, dataSize);

    outfile.write(reinterpret_cast<const char *>(audioData.data()), dataSize);

    outfile.close();

    std::cout << "Successfully wrote " << dataSize
              << " bytes of WAV audio data to: " << filename << std::endl;

    return 0;
}