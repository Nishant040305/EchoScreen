#include "constants.h"

std::vector<NetworkEntry> networkList;
int status = 0;
int displayStart = 0;
int selectedIndex = 0;
NetworkEntry* NetworkSSID = nullptr;

SocketIOclient socketIO;
String userId = "";
String roomId = "";
bool socketConnected = false;
std::vector<Message> messages;
bool isCreator = false;
String URL = "esp-32-morse-based-chat-application.onrender.com";
WebSocketsClient webSocket;
