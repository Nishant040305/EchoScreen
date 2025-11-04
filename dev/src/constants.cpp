#include "constants.h"

std::vector<NetworkEntry> networkList;
int status = 0;
int displayStart = 0;
int selectedIndex = 0;
NetworkEntry *NetworkSSID = nullptr;

String userId = "";
String roomId = "";
bool socketConnected = false;
std::vector<Message> messages;
bool isCreator = false;
String URL = "10.79.73.187:4000";
WebSocketsClient webSocket;
String trans = "";
bool newMR = false;