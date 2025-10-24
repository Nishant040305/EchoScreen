#ifndef WEB_SOCKET
#define WEB_SOCKET
#include<constants.h>
#include<LiquidCrystal.h>
#include<sendConnectedUser.h>
extern LiquidCrystal lcd;
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
#endif