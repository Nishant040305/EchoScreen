#include <WiFi.h>
#include <LiquidCrystal_I2C.h>

// Adjust I2C address if needed (commonly 0x27 or 0x3F)
extern LiquidCrystal_I2C lcd;  

void setupSearchMode() {
  WiFi.mode(WIFI_AP);  // Enable Access Point mode
  const char* ssid = "ESP32_SEARCH_MODE";
  const char* password = "12345678";  // optional, can be empty

  if (WiFi.softAP(ssid, password)) {
    String mac = WiFi.softAPmacAddress();

    Serial.println("ESP32 is now discoverable in Search Mode!");
    Serial.print("Device MAC Address: ");
    Serial.println(mac);

    // Show status on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Discoverable Mode");

    lcd.setCursor(0, 1);
    lcd.print("SSID: ");
    lcd.print(ssid);

    lcd.setCursor(0, 2);
    lcd.print("Pass: ");
    lcd.print(password);

    lcd.setCursor(0, 3);
    lcd.print("MAC:");
    lcd.print(mac);
  } else {
    Serial.println("Failed to enable Search Mode.");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Search Mode Error!");
  }
}
