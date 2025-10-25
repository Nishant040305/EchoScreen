#include <Arduino.h>
#include <LiquidCrystal.h>
#include <constants.h>

extern LiquidCrystal lcd;

int pos = 0;
bool codeFormed = false;
bool codeComplete = false;
bool CAPS = false;
String dig4code = "";

const int visibleRows = 3;    
const int charsPerRow = 10; 
const int visibleChars = visibleRows * charsPerRow;
int topIndex = 0; 

// --- Character sets ---
const int totalChars = 62;
char characters[totalChars] = {
  'A','a','B','b','C','c','D','d','E','e',
  'F','f','G','g','H','h','I','i','J','j',
  'K','k','L','l','M','m','N','n','O','o',
  'P','p','Q','q','R','r','S','s','T','t',
  'U','u','V','v','W','w','X','x','Y','y',
  'Z','z','0','1','2','3','4','5','6','7','8','9'
};

// --- CAPS character set ---
const int CAPStotalChars = 36;
char CAPScharacters[CAPStotalChars] = {
  'A','B','C','D','E','F','G','H','I','J',
  'K','L','M','N','O','P','Q','R','S','T',
  'U','V','W','X','Y','Z',
  '0','1','2','3','4','5','6','7','8','9'
};

void drawCharacterGrid() {
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print("Code: " + dig4code);
  
  int charIndex = topIndex;
  int limit = CAPS ? CAPStotalChars : totalChars;
  char* activeSet = CAPS ? CAPScharacters : characters;
  
  for (int row = 0; row < visibleRows; row++) {
    lcd.setCursor(0, row + 1);
    for (int col = 0; col < charsPerRow && charIndex < limit; col++) {
      lcd.print(activeSet[charIndex]);
      lcd.print(' ');
      charIndex++;
    }
  }
}

void getCursorPosition(int charPos, int &lcdX, int &lcdY) {
  int relativePos = charPos - topIndex;
  int row = relativePos / charsPerRow;
  int col = relativePos % charsPerRow;
  
  lcdX = col * 2;  
  lcdY = row + 1;  
}

void highlightCursor() {
  int limit = CAPS ? CAPStotalChars : totalChars;

  if (pos < 0) pos = limit - 1;
  if (pos >= limit) pos = 0;
  
  int currentBlock = pos / visibleChars;
  int displayBlock = topIndex / visibleChars;
  
  if (currentBlock != displayBlock) {
    topIndex = currentBlock * visibleChars;
    drawCharacterGrid();
  }
  
  int lcdX, lcdY;
  getCursorPosition(pos, lcdX, lcdY);
  lcd.setCursor(lcdX, lcdY);
  lcd.blink();
}

void formCode() {
  dig4code = "";
  pos = 0;
  topIndex = 0;
  codeFormed = true;
  codeComplete = false;
  
  drawCharacterGrid();
  highlightCursor();
  Serial.println("Character grid displayed");
}

// --- Move left/right with wrap and block scrolling ---
void moveLeft() {
  int limit = CAPS ? CAPStotalChars : totalChars;
  pos--;
  if (pos < 0) pos = limit - 1;
  highlightCursor();
}

void moveRight() {
  int limit = CAPS ? CAPStotalChars : totalChars;
  pos++;
  if (pos >= limit) pos = 0;
  highlightCursor();
}

// --- Append selected character ---
void appendCharToCode(int digits = 4) {
  if (codeComplete) return;
  
  char* activeSet = CAPS ? CAPScharacters : characters;
  dig4code += activeSet[pos];
  
  drawCharacterGrid();
  highlightCursor();
  
  if (dig4code.length() >= digits) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Code complete!");
    lcd.setCursor(0, 1);
    lcd.print("Code: " + dig4code);
    lcd.noBlink();
    codeComplete = true;
    Serial.println("Code completed: " + dig4code);
    delay(1000);
  }
}

void codeInputinit(bool config){
  pos = 0;
  codeFormed = false;
  codeComplete = false;
  dig4code = "";
  topIndex = 0;
  CAPS = config;
}

// --- Handle buttons ---
bool handleCodeInput(String& code,int digits = 4) {
  int b1 = digitalRead(BTN1); // select/enter
  int b2 = digitalRead(BTN2); // clear
  int b3 = digitalRead(BTN3); // left
  int b4 = digitalRead(BTN4); // right
  
  if (b1 == LOW) {
    if (!codeFormed) formCode();
    else appendCharToCode(digits);
    delay(300);
  }
  
  if (b2 == LOW && codeFormed && !codeComplete) {
    if (dig4code.length() > 0) {
      dig4code.remove(dig4code.length() - 1);
    }
    drawCharacterGrid();
    highlightCursor();
    delay(150);
  }
  
  if (b3 == LOW && codeFormed && !codeComplete) {
    moveLeft();
    delay(150);
  }

  if (b4 == LOW && codeFormed && !codeComplete) {
    moveRight();
    delay(150);
  }

  if (codeFormed && !codeComplete) {
    highlightCursor();
    return true;
  }
  
  if (codeComplete) {
    code = dig4code;
  }
  return !codeComplete;
}
