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
String URL = "echoscreen.onrender.com";
WebSocketsClient webSocket;
String trans = "";
bool newMR = false;