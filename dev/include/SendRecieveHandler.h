#include <constants.h>
#include <LiquidCrystal.h>
extern LiquidCrystal lcd;
void sendCreateRoom();
void sendJoinRoom(String roomName);
void sendAudioTranscription(String &audioFile);