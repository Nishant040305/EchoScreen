#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <Arduino.h>
#include <vector>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
// Pin definitions
const int BTN1 = 12;
const int BTN2 = 13;
const int BTN3 = 14;
const int BTN4 = 27;
// LCD
const int LCD_D7 = 4;
const int LCD_D5 = 15;
const int LCD_D6 = 2;
const int LCD_D4 = 21;
const int LCD_RS = 18;
const int LCD_E = 19;
// microphone i2s
const int I2S_SCK = 32;
const int I2S_SD = 34;
const int I2S_WS = 35;

typedef struct NetworkEntry
{
  String ssid;
  String mac;
} NetworkEntry;

extern std::vector<NetworkEntry> networkList;
extern int status;
extern int displayStart;
extern int selectedIndex;
extern NetworkEntry *NetworkSSID;
// socket
extern WebSocketsClient webSocket;

extern String userId;
extern bool socketConnected;
extern String roomId;
extern bool isCreator;
struct Message
{
  String type;
  String text;
  unsigned long timestamp;
};
extern String URL;
extern std::vector<Message> messages;
extern String trans;
extern bool newMR;
#endif
