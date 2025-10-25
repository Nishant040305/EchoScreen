#ifndef CONSTANTS_H
#define CONSTANTS_H
#include<Arduino.h>
#include<vector>
#include<SocketIOclient.h>
#include <ArduinoJson.h>
#include<WebSocketsClient.h>
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
//socket
extern WebSocketsClient webSocket;

extern SocketIOclient socketIO;
extern String userId;
extern bool socketConnected;
extern String roomId;
extern bool isCreator;
struct Message {
  String type;
  String text;
  unsigned long timestamp;
};
extern String URL;
extern std::vector<Message> messages;
extern String trans;
extern bool newMR;
#endif
