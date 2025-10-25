#ifndef HANDLE_CODE_INPUT_H
#define HANDLE_CODE_INPUT_H

#include<Arduino.h>
#include<constants.h>
#include <LiquidCrystal.h>
void formCode();
void highlightCursor();
void moveLeft();
void moveRight();
void appendCharToCode(int);
bool handleCodeInput(String& code,int digits);
void drawCharacterGrid();
void codeInputinit(bool);
#endif