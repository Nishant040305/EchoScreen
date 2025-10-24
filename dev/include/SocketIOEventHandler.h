#include<SocketIOclient.h>
#include<constants.h>
#include<LiquidCrystal.h>
#include<ArduinoJson.h>
extern LiquidCrystal lcd;
void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length);