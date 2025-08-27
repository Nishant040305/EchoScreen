/*
  Function: setupEspNowSender
  ---------------------------
  Initializes the ESP-NOW protocol and registers a peer device using the given MAC address.

  Parameters:
    - peerMac: A pointer to a 6-byte array representing the MAC address of the peer.

  Returns:
    - true if initialization and peer registration succeed.
    - false if any step fails (e.g., ESP-NOW init or peer registration).

  Behavior:
    - Sets WiFi mode to WIFI_STA.
    - Initializes ESP-NOW.
    - Registers a send callback to monitor send status.
    - Adds the peer device to the ESP-NOW peer list.

  Usage:
    const uint8_t peerMac[] = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0xCC};
    bool success = setupEspNowSender(peerMac);
*/
bool setupEspNowSender(const uint8_t* peerMac) {
  WiFi.mode(WIFI_STA);  // Required for ESP-NOW

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return false;
  }

  esp_now_register_send_cb(onDataSent);

  // Prepare peer configuration
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerMac, 6);
  peerInfo.channel = 0;         // Use current channel
  peerInfo.encrypt = false;     // No encryption by default

  if (!esp_now_is_peer_exist(peerMac)) {
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      Serial.println("Failed to add peer");
      return false;
    }
  }

  Serial.println("ESP-NOW setup complete.");
  return true;
}
