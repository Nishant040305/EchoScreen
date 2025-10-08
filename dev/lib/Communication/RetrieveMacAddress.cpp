/*
  Function: scanNearbyDevices
  ---------------------------
  Scans for nearby Wi-Fi-enabled devices (including ESP32 devices) and prints
  their MAC addresses (BSSID), signal strength (RSSI), and SSID (if available).

  Purpose:
    - Helps identify nearby ESP32 devices (or any Wi-Fi devices).
    - Useful for discovering potential ESP-NOW peers.

  Behavior:
    - Uses passive scanning to detect even hidden SSIDs.
    - Prints all results to the Serial Monitor.
    - Deletes scan results afterward to free memory.

  Requirements:
    - Must set WiFi mode to WIFI_STA before calling this function.
    - Call `WiFi.mode(WIFI_STA);` in `setup()`.

  Example Output:
    Scanning for nearby Wi-Fi devices...
    Found 2 device(s):
    [1] MAC: 24:6F:28:AA:BB:CC | RSSI: -40 dBm | SSID: ESP_NOW_DEVICE
    [2] MAC: 40:F5:20:11:22:33 | RSSI: -51 dBm | SSID: HiddenNetwork

  Usage:
    Call once in `setup()` or periodically in `loop()` for real-time discovery.

  Note:
    This function doesn't filter specifically for ESP devices,
    but you can look for known ESP32 MAC address prefixes like:
      - 24:6F:28
      - 7C:DF:A1
      - BC:DD:C2
*/

#include <WiFi.h>
#include <vector>

// Function to scan and collect nearby device MAC addresses
void scanNearbyDevices(std::vector<String>& macAddresses) {
  Serial.println("Scanning for nearby Wi-Fi devices...");
  
  int n = WiFi.scanNetworks(false, true);  // passive scan to detect hidden SSIDs
  if (n == 0) {
    Serial.println("No devices found.");
  } else {
    macAddresses = {};
    Serial.printf("Found %d device(s):\n", n);
    for (int i = 0; i < n; ++i) {
      String ssid = WiFi.SSID(i);
      String bssid = WiFi.BSSIDstr(i);
      int rssi = WiFi.RSSI(i);

      macAddresses.push_back(bssid);  // Add MAC to vector

      Serial.printf("[%d] MAC: %s | RSSI: %d dBm | SSID: %s\n", i + 1, bssid.c_str(), rssi, ssid.c_str());
    }
  }

  WiFi.scanDelete();  // Clear scan results to free memory
}
