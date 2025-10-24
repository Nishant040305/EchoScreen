#ifndef CONSTANTS_H
#define CONSTANTS_H
#include<Arduino.h>
#include<vector>
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
// Pin definitions
const int BTN1 = 12;
const int BTN2 = 13;
const int BTN3 = 14;
const int BTN4 = 27;

typedef struct NetworkEntry {
  String ssid;
  String mac;
} NetworkEntry;

extern std::vector<NetworkEntry> networkList;
extern int status;
extern int displayStart;
extern int selectedIndex;
extern NetworkEntry* NetworkSSID;
#endif
