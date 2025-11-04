#include<AudioHandler.h>
#include <ArduinoJson.h>

extern WebSocketsClient webSocket;
extern String userId;
extern String roomId;
extern bool socketConnected;

AudioHandler audioHandler;

AudioHandler::AudioHandler() {
    isRecording = false;
    isInitialized = false;
}

bool AudioHandler::init() {
    if (isInitialized) {
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
        .fixed_mclk = 0
    };

    const i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD
    };

    esp_err_t err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("Failed to install I2S driver: %d\n", err);
        return false;
    }

    err = i2s_set_pin(I2S_PORT, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("Failed to set I2S pins: %d\n", err);
        i2s_driver_uninstall(I2S_PORT);
        return false;
    }

    isInitialized = true;
    Serial.println("🎤 I2S Audio initialized successfully");
    return true;
}

void AudioHandler::startRecording() {
    if (!isInitialized) {
        Serial.println("❌ Audio not initialized");
        return;
    }

    if (isRecording) {
        Serial.println("⚠️ Already recording");
        return;
    }

    // Send start recording event to server
    StaticJsonDocument<200> doc;
    doc["userId"] = userId;
    doc["roomId"] = roomId;
    
    String payload;
    serializeJson(doc, payload);
    
    String message = "42[\"startAudioRecording\"," + payload + "]";
    webSocket.sendTXT(message);
    
    isRecording = true;
    i2s_start(I2S_PORT);
    
    Serial.println("🔴 Recording started");
}

void AudioHandler::stopRecording() {
    if (!isRecording) {
        return;
    }

    isRecording = false;
    i2s_stop(I2S_PORT);
    
    // Send stop recording event to server
    StaticJsonDocument<200> doc;
    doc["userId"] = userId;
    doc["roomId"] = roomId;
    
    String payload;
    serializeJson(doc, payload);
    
    String message = "42[\"stopAudioRecording\"," + payload + "]";
    webSocket.sendTXT(message);
    
    Serial.println("⏹️ Recording stopped");
}

void AudioHandler::processAudioChunk() {
    if (!isRecording || !socketConnected) {
        return;
    }

    int32_t samples[I2S_READ_LEN];
    size_t bytesRead = 0;

    esp_err_t result = i2s_read(I2S_PORT, samples, sizeof(samples), &bytesRead, 10);
    
    if (result == ESP_OK && bytesRead > 0) {
        // Create JSON wrapper for audio chunk
        StaticJsonDocument<128> doc;
        doc["userId"] = userId;
        doc["roomId"] = roomId;
        
        String jsonStr;
        serializeJson(doc, jsonStr);
        
        // Send as Socket.IO event with binary data
        String header = "452[\"audioChunk\"," + jsonStr + "]";
        
        // Calculate total size
        size_t headerLen = header.length();
        size_t totalLen = headerLen + bytesRead;
        
        // Allocate buffer
        uint8_t* buffer = (uint8_t*)malloc(totalLen);
        if (buffer) {
            memcpy(buffer, header.c_str(), headerLen);
            memcpy(buffer + headerLen, samples, bytesRead);
            
            webSocket.sendBIN(buffer, totalLen);
            free(buffer);
            
            Serial.printf("📤 Sent %d bytes\n", bytesRead);
        }
    }
}

void AudioHandler::cleanup() {
    if (isRecording) {
        stopRecording();
    }
    
    if (isInitialized) {
        i2s_driver_uninstall(I2S_PORT);
        isInitialized = false;
        Serial.println("🔇 I2S Audio cleaned up");
    }
}