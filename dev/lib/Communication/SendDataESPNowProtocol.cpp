/*
  Function: sendDataToPeer
  ------------------------
  Sends a message via ESP-NOW to a registered peer device.

  Parameters:
    - peerMac: A pointer to a 6-byte MAC address of the peer.
    - data: Pointer to the byte array to send.
    - len: Length of the data in bytes.

  Returns:
    - true if the data is sent successfully.
    - false if sending fails.

  Behavior:
    - Sends data using ESP-NOW's esp_now_send().
    - Uses a callback to print the send status (success/fail).

  Usage:
    const char* message = "Hello ESP-NOW";
    sendDataToPeer(peerMac, (const uint8_t*)message, strlen(message));
*/
bool sendDataToPeer(const uint8_t* peerMac, const uint8_t* data, size_t len) {
  esp_err_t result = esp_now_send(peerMac, data, len);
  if (result == ESP_OK) {
    Serial.println("Sent successfully");
    return true;
  } else {
    Serial.print("Send failed. Error code: ");
    Serial.println(result);
    return false;
  }
}
/*
  Callback: onDataSent
  ---------------------
  A callback function called after each ESP-NOW send attempt.

  Parameters:
    - mac_addr: MAC address to which data was sent.
    - status: Indicates if the send operation was successful or failed.

  Behavior:
    - Prints the status of the last send to the Serial Monitor.
*/
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}
