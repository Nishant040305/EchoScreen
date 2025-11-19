// AudioModule.cpp
#include <AudioModule.h>
#include <WiFi.h>
#include <driver/i2s.h>
#include <WebSocketsClient.h>
#include<constants.h>
#define I2S_PORT I2S_NUM_0

#define I2S_READ_LEN   1024
#define RING_BUFFER_SIZE  (I2S_READ_LEN * 16)

static int32_t ringBuffer[RING_BUFFER_SIZE];
volatile size_t writeIndex = 0, readIndex = 0, bufferedSamples = 0;

WebSocketsClient webSocketJava;
bool javaSocketConnected = false;

const char* javaServerHost = "10.105.43.187";
const uint16_t javaServerPort = 8081;
const char* javaServerPath = "/";

void i2sReaderTask(void *parameter);

void initI2SMic() {
  const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 128,
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

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_start(I2S_PORT);

  xTaskCreatePinnedToCore(i2sReaderTask, "i2sReader", 12288, NULL, 2, NULL, 0);

  Serial.println("🎤 I2S microphone initialized.");
}
void startJavaWebSocket() {
  Serial.println("Connecting to Java server...");
  webSocketJava.begin(javaServerHost, javaServerPort, javaServerPath);
  webSocketJava.onEvent([](WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
      case WStype_CONNECTED:
        javaSocketConnected = true;
        Serial.println("✅ Connected to Java audio server!");
        break;
      case WStype_DISCONNECTED:
        javaSocketConnected = false;
        Serial.println("❌ Java server disconnected!");
        break;
      case WStype_ERROR:
        Serial.println("⚠️ WebSocket error with Java server");
        break;
      default:
        break;
    }
  });
  webSocketJava.setReconnectInterval(5000);
}

void i2sReaderTask(void *parameter) {
  int32_t samples[I2S_READ_LEN];
  size_t bytesRead;
  while (true) {
    if (i2s_read(I2S_PORT, (void*)samples, sizeof(samples), &bytesRead, portMAX_DELAY) == ESP_OK) {
      int numSamples = bytesRead / sizeof(int32_t);
      for (int i = 0; i < numSamples; i++) {
        ringBuffer[writeIndex] = samples[i];
        writeIndex = (writeIndex + 1) % RING_BUFFER_SIZE;
        if (bufferedSamples < RING_BUFFER_SIZE)
          bufferedSamples++;
        else
          readIndex = (readIndex + 1) % RING_BUFFER_SIZE;
      }
    }
  }
}

void handleJavaWebSocketLoop() {
  webSocketJava.loop();
}

void sendAudioToJava() {
  extern String userId;
  extern String roomId;  // Add this extern declaration

  if (!javaSocketConnected || userId == "" || roomId == "" || bufferedSamples < I2S_READ_LEN)
    return;

  int32_t tempBuf[I2S_READ_LEN];
  for (int i = 0; i < I2S_READ_LEN; i++) {
    tempBuf[i] = ringBuffer[readIndex];
    readIndex = (readIndex + 1) % RING_BUFFER_SIZE;
  }
  bufferedSamples -= I2S_READ_LEN;

  const int USER_ID_LENGTH = 8; 
  const int ROOM_ID_LENGTH = 4;
  
  // Prepare userId header (8 bytes)
  char idHeader[USER_ID_LENGTH];
  memset(idHeader, ' ', USER_ID_LENGTH); 
  strncpy(idHeader, userId.c_str(), USER_ID_LENGTH);
  
  // Prepare roomId header (4 bytes)
  char roomHeader[ROOM_ID_LENGTH];
  memset(roomHeader, ' ', ROOM_ID_LENGTH);
  strncpy(roomHeader, roomId.c_str(), ROOM_ID_LENGTH);
  
  // Calculate total data size: userId (8) + roomId (4) + audio data
  size_t dataSize = USER_ID_LENGTH + ROOM_ID_LENGTH + I2S_READ_LEN * sizeof(int32_t);
  uint8_t *sendBuffer = (uint8_t *)malloc(dataSize);
  if (!sendBuffer) return; // fail-safe

  // Copy data: [userId][roomId][audio]
  memcpy(sendBuffer, idHeader, USER_ID_LENGTH);
  memcpy(sendBuffer + USER_ID_LENGTH, roomHeader, ROOM_ID_LENGTH);
  memcpy(sendBuffer + USER_ID_LENGTH + ROOM_ID_LENGTH, (uint8_t *)tempBuf, I2S_READ_LEN * sizeof(int32_t));

  webSocketJava.sendBIN(sendBuffer, dataSize);
  free(sendBuffer);

  Serial.printf("📤 Sent %d samples (%d bytes + %dB userId + %dB roomId) to Java [User:%s, Room:%s]\n",
                I2S_READ_LEN, I2S_READ_LEN * 4, USER_ID_LENGTH, ROOM_ID_LENGTH, 
                userId.c_str(), roomId.c_str());
}