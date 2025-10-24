#include <vector>
#include<Arduino.h>
#include <LiquidCrystal.h>
#include <WiFi.h>
#include<constants.h>
typedef struct NetworkEntry {
  String ssid;
  String mac;
} NetworkEntry;
extern LiquidCrystal lcd;  
std::vector<NetworkEntry> networkList;
int selectedIndex = 0;
int displayStart = 0;
extern int status;
void displayNetworks() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Mode: Display");

  // Show up to 3 networks starting at displayStart
  for (int i = 0; i < 3; i++) {
    int idx = displayStart + i;
    if (idx < (int)networkList.size()) {
      lcd.setCursor(0, i + 1);

      // Mark selected entry with ">"
      if (idx == selectedIndex) {
        lcd.print(">");
      } else {
        lcd.print(" ");
      }

      // Show SSID only (truncate if too long)
      String entry = networkList[idx].ssid;
      lcd.print(entry.substring(0, 18));  // fit into 20 chars with marker
    }
  }
  delay(1000);
}

void setupFindDevice() {
  WiFi.mode(WIFI_STA);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Mode: Scanner");
  lcd.setCursor(0, 1);
  lcd.print("Scanning...");

  Serial.println("Starting WiFi scan...");
  int n = WiFi.scanNetworks();

  networkList.clear();
  if (n == 0) {
    Serial.println("No networks found");
    lcd.setCursor(0, 2);
    lcd.print("No devices found");
  } else {
    Serial.printf("Found %d networks:\n", n);

    for (int i = 0; i < n; i++) {
      NetworkEntry entry;
      entry.ssid = WiFi.SSID(i);
      entry.mac = WiFi.BSSIDstr(i);
      networkList.push_back(entry);

      Serial.printf("%d: %s  MAC: %s  RSSI: %d\n",
                    i + 1,
                    entry.ssid.c_str(),
                    entry.mac.c_str(),
                    WiFi.RSSI(i));
    }

    selectedIndex = 0;
    displayStart = 0;
    status = 3;
    WiFi.scanDelete();
  }

}

void handleScrollButtons() {
  if (digitalRead(BTN3) == LOW) {  // Scroll down
    if (selectedIndex < (int)networkList.size() - 1) {
      selectedIndex++;
      if (selectedIndex >= displayStart + 3) {
        displayStart++;
      }
      displayNetworks();
    }
    delay(200);  // debounce
  }

  if (digitalRead(BTN4) == LOW) {  // Scroll up
    if (selectedIndex > 0) {
      selectedIndex--;
      if (selectedIndex < displayStart) {
        displayStart--;
      }
      displayNetworks();
    }
    delay(200);  // debounce
  }
}
