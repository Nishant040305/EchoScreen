#include <AudioHandler.h>
#include <ArduinoJson.h>

extern WebSocketsClient webSocket;
extern String userId;
extern String roomId;
extern bool socketConnected;

AudioHandler audioHandler;

AudioHandler::AudioHandler()
{
    isRecording = false;
    isInitialized = false;
    lastSendTime = 0;
    chunkCount = 0;
}

bool AudioHandler::init()
{
    if (isInitialized)
    {
        return true;
    }

    const i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = BITS_PER_SAMPLE,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 256,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0};

    const i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD};

    esp_err_t err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK)
    {
        Serial.printf("Failed to install I2S driver: %d\n", err);
        return false;
    }

    err = i2s_set_pin(I2S_PORT, &pin_config);
    if (err != ESP_OK)
    {
        Serial.printf("Failed to set I2S pins: %d\n", err);
        i2s_driver_uninstall(I2S_PORT);
        return false;
    }

    isInitialized = true;
    Serial.println("🎤 I2S Audio initialized successfully");
    return true;
}

void AudioHandler::startRecording()
{
    if (!isInitialized)
    {
        Serial.println("❌ Audio not initialized");
        return;
    }

    if (isRecording)
    {
        Serial.println("⚠️ Already recording");
        return;
    }

    // Format: 42["startAudioRecording", {"roomId":"xxx", "userId":"yyy"}]
    DynamicJsonDocument doc(512);
    JsonObject data = doc.to<JsonObject>();
    data["roomId"] = roomId;
    data["userId"] = userId;

    String jsonData;
    serializeJson(data, jsonData);

    String message = "42[\"startAudioRecording\"," + jsonData + "]";
    webSocket.sendTXT(message);

    isRecording = true;
    lastSendTime = 0;
    chunkCount = 0;
    i2s_start(I2S_PORT);

    Serial.println("🔴 Recording started: " + message);
}

void AudioHandler::stopRecording()
{
    if (!isRecording)
    {
        return;
    }

    isRecording = false;
    i2s_stop(I2S_PORT);

    // Format: 42["stopAudioRecording", {"roomId":"xxx", "userId":"yyy"}]
    DynamicJsonDocument doc(512);
    JsonObject data = doc.to<JsonObject>();
    data["roomId"] = roomId;
    data["userId"] = userId;

    String jsonData;
    serializeJson(data, jsonData);

    String message = "42[\"stopAudioRecording\"," + jsonData + "]";
    webSocket.sendTXT(message);

    Serial.printf("⏹️ Recording stopped. Total chunks sent: %d\n", chunkCount);
}

void AudioHandler::processAudioChunk()
{
    if (!isRecording || !socketConnected)
    {
        return;
    }

    // Rate limiting: Send chunks every 100ms (10 chunks/sec)
    unsigned long currentTime = millis();
    if (currentTime - lastSendTime < 100)
    {
        return;
    }

    int32_t samples[I2S_READ_LEN];
    size_t bytesRead = 0;

    esp_err_t result = i2s_read(I2S_PORT, samples, sizeof(samples), &bytesRead, 10);

    if (result == ESP_OK && bytesRead > 0)
    {
        // Convert 32-bit samples to 16-bit PCM
        size_t sampleCount = bytesRead / sizeof(int32_t);
        int16_t *pcm16 = (int16_t *)malloc(sampleCount * sizeof(int16_t));

        if (!pcm16)
        {
            Serial.println("❌ Memory allocation failed");
            return;
        }

        for (size_t i = 0; i < sampleCount; i++)
        {
            // Convert 32-bit to 16-bit by right-shifting
            pcm16[i] = (int16_t)(samples[i] >> 16);
        }

        size_t pcm16Size = sampleCount * sizeof(int16_t);

        // Format: 451-["audioChunk", {"roomId":"xxx", "userId":"yyy"}]
        DynamicJsonDocument doc(512);
        JsonObject data = doc.to<JsonObject>();
        data["roomId"] = roomId;
        data["userId"] = userId;

        String jsonData;
        serializeJson(data, jsonData);

        // Socket.IO binary event format
        String header = "451-[\"audioChunk\"," + jsonData + "]";

        size_t headerLen = header.length();
        size_t totalLen = headerLen + pcm16Size;

        // Allocate buffer for header + binary data
        uint8_t *buffer = (uint8_t *)malloc(totalLen);

        if (buffer)
        {
            memcpy(buffer, header.c_str(), headerLen);
            memcpy(buffer + headerLen, pcm16, pcm16Size);

            webSocket.sendBIN(buffer, totalLen);
            free(buffer);

            chunkCount++;
            lastSendTime = currentTime;

            // Log every 10th chunk to reduce serial spam
            if (chunkCount % 10 == 0)
            {
                Serial.printf("📤 Sent %d chunks (last: %d bytes)\n", chunkCount, pcm16Size);
            }
        }
        else
        {
            Serial.println("❌ Buffer allocation failed");
        }

        free(pcm16);
    }
}

void AudioHandler::cleanup()
{
    if (isRecording)
    {
        stopRecording();
    }

    if (isInitialized)
    {
        i2s_driver_uninstall(I2S_PORT);
        isInitialized = false;
        Serial.println("🔇 I2S Audio cleaned up");
    }
}