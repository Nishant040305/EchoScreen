//Buttons
#include <constants.h>
/*
BTN1 - Button 1
This button is connected to GPIO 12.
Pressing this button adds '.' character into the buffer.

BTN2 - Button 2
This button is connected to GPIO 13.
Pressing this button adds '_' character into the buffer.

BTN3 - Button 3
This button is connected to GPIO 14.

Scenario1: Morse Buffer is not empty
Pressing this button converts the buffer into the equivalent character ands to the message.
and empty the buffer.

Scenario2: Morse Buffer is empty
Pressing this button adds a space ' ' character into the message.
and buffer remains empty.

BTN4 - Button 4
This button is connected to GPIO 27.
Pressing this button clears both the buffer and the message.

*/

//Process -1 Initialize Components
#include <LiquidCrystal.h>
#include <vector>
#include <WiFi.h>
#include <esp_now.h>
#include <showLandingScreen.h>
#include <handlerButtonMenu.h>
#include <handlButtonNetworkMenu.h>
#include <setupFindDevice.h>
#include <handleCodeInput.h>
LiquidCrystal lcd(19, 23, 18, 17, 16, 15);
void setup(){
  lcd.begin(20,4);
  Serial.begin(115200);
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);
  pinMode(BTN4, INPUT_PULLUP);
  showLandingScreen();
}
void loop(){
  switch (status) {
    case 0: // Landing Screen
      showLandingScreen();
      while (handlerButtonMenu());
      break;
    case 1:
      setupFindDevice();
      break;
    case 3: // Network List
      displayNetworks();
      handleNetworkMenu();
      break;
    case 4:
      if(!NetworkSSID){status = 1;}
      else{
        NetworkEntry entry = *NetworkSSID;
        Serial.printf("SSID: %s\n",entry.ssid.c_str());
        // Serial.printf("SSID: %s\n",entry.mac.c_str());
        lcd.clear();
        lcd.print("Press BTN1 for Pass");
        String code = "";
        while(handleCodeInput(code,4));
        status=5;
      }
    case 5:
      lcd.clear();
      lcd.print("Reserved Mode");
      delay(1000);
      break;

    default:
      status = 0;
      break;
  }

}

