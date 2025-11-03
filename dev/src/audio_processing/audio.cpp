// #include <Arduino.h>
// #include <WiFi.h>
// #include <WebSocketsClient.h>
// #include <driver/i2s.h>

// // ====== I2S MIC PIN CONFIG ======
// #define I2S_WS   12
// #define I2S_SD   32
// #define I2S_SCK  14
// #define I2S_PORT I2S_NUM_0

// // // ====== WIFI CONFIG ======
// // // const char* ssid = "niggas only";
// // // const char* password = "namanokok";
// // const char* ssid = "One Plus Nord 5";
// // const char* password = "aserfgbn";

// // ====== WEBSOCKET CONFIG ======
// WebSocketsClient webSocket;
// const char* ws_host = "10.79.73.139";
// const uint16_t ws_port = 8080;
// const char* ws_path = "/";

// // ====== RING BUFFER CONFIG ======
// #define I2S_READ_LEN   1024              // samples per read
// #define RING_BUFFER_SIZE  (I2S_READ_LEN * 16) // 16 chunks of 1024 samples

// static int32_t ringBuffer[RING_BUFFER_SIZE];
// volatile size_t writeIndex = 0;
// volatile size_t readIndex = 0;
// volatile size_t bufferedSamples = 0;
// portMUX_TYPE bufferMux = portMUX_INITIALIZER_UNLOCKED;

// // ====== Task Declaration ======
// void i2sReaderTask(void *parameter);

// // ====== Setup ======
// void setup() {
//   Serial.begin(115200);
//   delay(500);
//   Serial.println("🎤 ESP32 I2S → RingBuffer → WebSocket");

//   // --- Connect to WiFi ---
//   WiFi.begin(ssid, password);
//   Serial.print("Connecting to WiFi");
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(300);
//     Serial.print(".");
//   }
//   Serial.println("\n✅ Connected!");
//   Serial.print("IP: "); Serial.println(WiFi.localIP());

//   // --- Setup WebSocket ---
//   webSocket.begin(ws_host, ws_port, ws_path);
//   webSocket.onEvent([](WStype_t type, uint8_t *payload, size_t length) {
//     switch (type) {
//       case WStype_CONNECTED:
//         Serial.println("✅ WebSocket Connected!");
//         break;
//       case WStype_DISCONNECTED:
//         Serial.println("❌ WebSocket Disconnected!");
//         break;
//       case WStype_ERROR:
//         Serial.println("⚠️ WebSocket Error!");
//         break;
//       default:
//         break;
//     }
//   });
//   webSocket.setReconnectInterval(5000);

//   const i2s_config_t i2s_config = {
//     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
//     .sample_rate = 16000,
//     .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
//     .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
//     .communication_format = I2S_COMM_FORMAT_I2S,
//     .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
//     .dma_buf_count = 8,
//     .dma_buf_len = 128,
//     .use_apll = false,
//     .tx_desc_auto_clear = false,
//     .fixed_mclk = 0
//   };

//   const i2s_pin_config_t pin_config = {
//     .bck_io_num = I2S_SCK,
//     .ws_io_num = I2S_WS,
//     .data_out_num = I2S_PIN_NO_CHANGE,
//     .data_in_num = I2S_SD
//   };

//   i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
//   i2s_set_pin(I2S_PORT, &pin_config);
//   i2s_start(I2S_PORT);

//   xTaskCreatePinnedToCore(i2sReaderTask, "i2sReader", 4096, NULL, 2, NULL, 0);
// }
// void loop() {
//   webSocket.loop();

//   if (webSocket.isConnected() && bufferedSamples >= I2S_READ_LEN) {
//     int32_t tempBuf[I2S_READ_LEN];
//     for (int i = 0; i < I2S_READ_LEN; i++) {
//       tempBuf[i] = ringBuffer[readIndex];
//       readIndex = (readIndex + 1) % RING_BUFFER_SIZE;
//     }
//     bufferedSamples -= I2S_READ_LEN;
//     webSocket.sendBIN((uint8_t*)tempBuf, I2S_READ_LEN * sizeof(int32_t));
//     Serial.printf("📤 Sent %d samples (%d bytes), buffered=%d\n",
//                   I2S_READ_LEN, I2S_READ_LEN * 4, (int)bufferedSamples);
//   }

//   delay(2); // small yield
// }
// int32_t samples[I2S_READ_LEN];
// // ====== Reader Task ======
// void i2sReaderTask(void *parameter) {

//   size_t bytesRead;

//   Serial.println("🎧 I2S Reader Task Started");

//   while (true) {
//     esp_err_t res = i2s_read(I2S_PORT, (void*)samples, sizeof(samples), &bytesRead, portMAX_DELAY);
//     if (res == ESP_OK && bytesRead > 0) {
//       int num_samples = bytesRead / sizeof(int32_t);
//       for (int i = 0; i < num_samples; i++) {
//         ringBuffer[writeIndex] = samples[i];
//         writeIndex = (writeIndex + 1) % RING_BUFFER_SIZE;
//         if (bufferedSamples < RING_BUFFER_SIZE)
//           bufferedSamples++;
//         else
//           readIndex = (readIndex + 1) % RING_BUFFER_SIZE;
//       }
//     }
//   }
// }
