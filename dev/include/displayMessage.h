#ifndef DISPLAY_MESSAGE_H
#define DISPLAY_MESSAGE_H
#include<constants.h>
#include<LiquidCrystal.h>
extern LiquidCrystal lcd;
void buildFullText();
void renderMessages();
void handleScrollUp();
void handleScrollDown();
bool handleExitMessages();
void handleNewMessage();
void initMessageDisplay();
void wrapText();
#endif