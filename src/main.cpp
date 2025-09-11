//Buttons
#define B1 12
#define B2 13
#define B3 14
#define B4 27
int status = 0;
/*
B1 - Button 1
This button is connected to GPIO 12.
Pressing this button adds '.' character into the buffer.

B2 - Button 2
This button is connected to GPIO 13.
Pressing this button adds '_' character into the buffer.

B3 - Button 3
This button is connected to GPIO 14.

Scenario1: Morse Buffer is not empty
Pressing this button converts the buffer into the equivalent character ands to the message.
and empty the buffer.

Scenario2: Morse Buffer is empty
Pressing this button adds a space ' ' character into the message.
and buffer remains empty.

B4 - Button 4
This button is connected to GPIO 27.
Pressing this button clears both the buffer and the message.

*/

//Process -1 Initialize LCD
#include <LiquidCrystal.h>
#include <vector>
#include<WiFi.h>
#include <esp_now.h>

LiquidCrystal lcd(19, 23, 18, 17, 16, 15);

//Interfacing
void showMenu();
void setupSearchMode();
void setupFindDevice();
void handleScrollButtons();
bool handlerButtonMenu();
void displayNetworks();
void handleChatMode();
void updateChatLCD();
void MorseCodeChatApplication();
char morseToChar(String);
struct NetworkEntry {
  String ssid;
  String mac;
};

std::vector<NetworkEntry> networkList;
int selectedIndex = 0;   // which network is currently selected
int displayStart = 0;    // where the display window starts
String morseBuffer = "";   // stores current Morse code
String message = "";       // full decoded message
unsigned long lastPressB3 = 0;


NetworkEntry activePeer;   // The peer currently selected for chat
bool peerSelected = false; // Track whether we have a peer

void setup(){
  lcd.begin(20,4);
  Serial.begin(115200);
  pinMode(B1, INPUT_PULLUP);
  pinMode(B2, INPUT_PULLUP);
  pinMode(B3, INPUT_PULLUP);
  pinMode(B4, INPUT_PULLUP);
}
void loop(){
  if(status==0) {
    showMenu();
    while(handlerButtonMenu());
  }else if(status==1){
    setupSearchMode();
  }else if(status ==2){
    setupFindDevice();
  }else if(status ==3){
    displayNetworks();
    while((digitalRead(B3)==HIGH)&&(digitalRead(B4)==HIGH)){
      handleScrollButtons();
      delay(500);
    }

    if(digitalRead(B3)==LOW){
      status = 0;
    }
  }else if(status == 4){
    //selected for chat

  }else if(status ==5){
    MorseCodeChatApplication();
  }

}
//Menu Handler
bool handlerButtonMenu(){
  if (digitalRead(B1) == LOW) {
    status = 1;
    delay(500);
    return false;
  }

  if (digitalRead(B2) == LOW) {
    status = 2;
    delay(500);
    return false;
  }

  if (digitalRead(B4) == LOW) {  
      status = 5;
      return false;
    }
  
  if(digitalRead(B3)==LOW){
    status = 0;
    return false;
  }
  return true;
}
//Shows the menu of the Initial Operation
void showMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Choose Mode:");
  lcd.setCursor(0, 1);
  lcd.print("B1=Discoverable/ B3=Reset ");
  lcd.setCursor(0, 2);
  lcd.print("B2=Scan Mode");
  lcd.setCursor(0, 3);
  lcd.print("B4=Morse code Editor");
}

//Discover Mode
void setupSearchMode() {
  WiFi.mode(WIFI_AP); 
  const char* ssid = "ESP32_SEARCH_MODE"; 
  const char* password = ""; // Empty = open network

  // Start AP with open authentication
  bool result = WiFi.softAP(ssid, password, 1, 0, 4);

  if (result) {
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
    lcd.print("Pass: OPEN");

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


void displayNetworks() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Mode: Scanner");
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
    status = 3;
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
  }

  WiFi.scanDelete();
}

void handleScrollButtons() {
  if (digitalRead(B1) == LOW) {  // Scroll down
    Serial.printf("Scroll Down");
    if (selectedIndex < (int)networkList.size() - 1) {
      selectedIndex++;
      if (selectedIndex >= displayStart + 3) {
        displayStart++;
      }
      displayNetworks();
    }
    delay(200);  // debounce
  }

  if (digitalRead(B2) == LOW) {  // Scroll up
  Serial.printf("Scroll Up");
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

// bool setupEspNowSender(const String& macStr);
// bool sendDataToPeer(const String& macStr, const uint8_t* data, size_t len);
// void onDataSent(const uint8_t* mac_addr, esp_now_send_status_t status);

// // ============================
// // Convert "AA:BB:CC:DD:EE:FF" → uint8_t[6]
// // ============================
// bool macStringToBytes(const String& macStr, uint8_t* macBytes) {
//   if (macStr.length() != 17) return false; // Must be XX:XX:XX:XX:XX:XX
//   int values[6];
//   if (sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x",
//              &values[0], &values[1], &values[2],
//              &values[3], &values[4], &values[5]) == 6) {
//     for (int i = 0; i < 6; i++) {
//       macBytes[i] = (uint8_t) values[i];
//     }
//     return true;
//   }
//   return false;
// }

// // ============================
// // Setup ESP-NOW sender
// // ============================
// bool setupEspNowSender(const String& macStr) {
//   uint8_t peerMac[6];
//   if (!macStringToBytes(macStr, peerMac)) {
//     Serial.println("Invalid MAC format!");
//     return false;
//   }

//   WiFi.mode(WIFI_STA);

//   if (esp_now_init() != ESP_OK) {
//     Serial.println("Error initializing ESP-NOW");
//     return false;
//   }

//   esp_now_register_send_cb(onDataSent);

//   // Prepare peer info
//   esp_now_peer_info_t peerInfo = {};
//   memcpy(peerInfo.peer_addr, peerMac, 6);
//   peerInfo.channel = 0;
//   peerInfo.encrypt = false;

//   if (!esp_now_is_peer_exist(peerMac)) {
//     if (esp_now_add_peer(&peerInfo) != ESP_OK) {
//       Serial.println("Failed to add peer");
//       return false;
//     }
//   }

//   Serial.println("ESP-NOW setup complete.");
//   return true;
// }

// // ============================
// // Send data to peer
// // ============================
// bool sendDataToPeer(const String& macStr, const uint8_t* data, size_t len) {
//   uint8_t peerMac[6];
//   if (!macStringToBytes(macStr, peerMac)) {
//     Serial.println("Invalid MAC format!");
//     return false;
//   }

//   esp_err_t result = esp_now_send(peerMac, data, len);
//   if (result == ESP_OK) {
//     Serial.println("Send queued successfully");
//     return true;
//   } else {
//     Serial.print("Send failed. Error code: ");
//     Serial.println(result);
//     return false;
//   }
// }

// // ============================
// // Callback after each send
// // ============================
// void onDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
//   char macStr[18];
//   snprintf(macStr, sizeof(macStr),
//            "%02X:%02X:%02X:%02X:%02X:%02X",
//            mac_addr[0], mac_addr[1], mac_addr[2],
//            mac_addr[3], mac_addr[4], mac_addr[5]);

//   Serial.print("Last Packet to: ");
//   Serial.print(macStr);
//   Serial.print(" Status: ");
//   Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
// }


// String morseBuffer = "";
// String messageBuffer = "";

// void handleChatMode() {
//   if (digitalRead(B1) == LOW) {  // dot
//     morseBuffer += ".";
//     updateChatLCD();
//     delay(200);
//   }
//   if (digitalRead(B2) == LOW) {  // dash
//     morseBuffer += "_";
//     updateChatLCD();
//     delay(200);
//   }
//   if (digitalRead(B3) == LOW) {  
//     if (morseBuffer.length() > 0) {
//       // Convert Morse to letter (you’ll need a lookup function)
//       char letter = morseToChar(morseBuffer);
//       messageBuffer += letter;
//       morseBuffer = "";
//     } else {
//       messageBuffer += " ";
//     }
//     updateChatLCD();
//     delay(200);
//   }
//   if (digitalRead(B4) == LOW) {  // Clear buffers / send
//     if (messageBuffer.length() > 0 && peerSelected) {
//       sendDataToPeer(activePeer.mac, (const uint8_t*)messageBuffer.c_str(), messageBuffer.length());
//       Serial.println("Message sent: " + messageBuffer);
//       messageBuffer = "";
//       morseBuffer = "";
//     } else {
//       // Just clear
//       messageBuffer = "";
//       morseBuffer = "";
//     }
//     updateChatLCD();
//     delay(200);
//   }
// }

// void updateChatLCD() {
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print("Chat Mode");
//   lcd.setCursor(0, 1);
//   lcd.print("Msg: " + messageBuffer);
//   lcd.setCursor(0, 2);
//   lcd.print("Morse: " + morseBuffer);
// }
void MorseCodeChatApplication(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Morse Encoder");
  lcd.setCursor(0, 1);
  lcd.print("Msg: ");
  while(true){
    // B1 - dot
    if(digitalRead(B1) == LOW) {
      morseBuffer += ".";
      Serial.println(".");
      delay(300);
    }

    // B2 - dash
    if(digitalRead(B2) == LOW) {
      morseBuffer += "_";
      Serial.println("_");
      delay(300);
    }

    // B3 - get character
    if(digitalRead(B3) == LOW) {
      Serial.print("This");
      if(morseBuffer.length()==0){
        message+=' ';
      }else{
        char result = morseToChar(morseBuffer);
        message+=result;
        morseBuffer = "";
      }
      delay(300);
    }

    // B4 - enter (space or end)
    if(digitalRead(B4) == LOW) {
      message += " ";
      morseBuffer = "";
      Serial.println("Enter pressed, buffer cleared.");
      delay(300);
    }

    // Update LCD
    lcd.setCursor(0, 1);
    lcd.print("Msg: " + message + "        ");  // clear remaining
    lcd.setCursor(0, 2);
    lcd.print("Buffer: " + morseBuffer + "       ");
  }
}

char morseToChar(String code) {
  if(code == "._") return 'A';
  if(code == "_...") return 'B';
  if(code == "_._.") return 'C';
  if(code == "_..") return 'D';
  if(code == ".") return 'E';
  if(code == ".._.") return 'F';
  if(code == "__.") return 'G';
  if(code == "....") return 'H';
  if(code == "..") return 'I';
  if(code == ".___") return 'J';
  if(code == "_._") return 'K';
  if(code == "._..") return 'L';
  if(code == "__") return 'M';
  if(code == "_.") return 'N';
  if(code == "___") return 'O';
  if(code == ".__.") return 'P';
  if(code == "__._") return 'Q';
  if(code == "._.") return 'R';
  if(code == "...") return 'S';
  if(code == "_") return 'T';
  if(code == ".._") return 'U';
  if(code == "..._") return 'V';
  if(code == ".__") return 'W';
  if(code == "_.._") return 'X';
  if(code == "_.__") return 'Y';
  if(code == "__..") return 'Z';
  if(code == "_____") return '0';
  if(code == ".____") return '1';
  if(code == "..___") return '2';
  if(code == "...__") return '3';
  if(code == "...._") return '4';
  if(code == ".....") return '5';
  if(code == "_....") return '6';
  if(code == "__...") return '7';
  if(code == "___..") return '8';
  if(code == "____.") return '9';
  return '?'; // unknown code
}
