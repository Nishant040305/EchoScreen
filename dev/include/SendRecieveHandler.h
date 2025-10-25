#include<constants.h>
#include<SocketIOclient.h>
#include<LiquidCrystal.h>
extern LiquidCrystal lcd;
void sendSocketEvent(String eventName, String data);
void sendSocketEventWithJson(String eventName, JsonObject data);
void sendCreateRoom();
void sendJoinRoom(String roomName);
void sendAudioTranscription(String& audioFile);