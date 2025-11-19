// AudioModule.h
#ifndef AUDIO_MODULE_H
#define AUDIO_MODULE_H

#include <Arduino.h>
#include <WebSocketsClient.h>

void initI2SMic();            
void startJavaWebSocket();      
void handleJavaWebSocketLoop();
void sendAudioToJava();

extern bool javaSocketConnected;
extern WebSocketsClient webSocketJava;
#endif
