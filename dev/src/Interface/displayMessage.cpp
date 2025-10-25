#include<constants.h>
#include<displayMessage.h>

String fullText = "";
std::vector<String> wrappedLines; // Store pre-wrapped lines
int currentRow = 0;
bool needsRedraw = true;

void buildFullText() {
  fullText = "";
  for(int i = 0; i < messages.size(); i++) {
    if(i > 0) fullText += " ";
    fullText += messages[i].text;
  }
}

void wrapText() {
  wrappedLines.clear();
  
  int pos = 0;
  int textLen = fullText.length();
  
  while(pos < textLen) {
    int remainingChars = textLen - pos;
    int chunkSize = min(20, remainingChars);
    
    // If we're not at the end and the chunk ends mid-word
    if(pos + chunkSize < textLen) {
      // Check if we're breaking a word
      if(fullText.charAt(pos + chunkSize) != ' ' && fullText.charAt(pos + chunkSize - 1) != ' ') {
        // Find the last space in the chunk
        int lastSpace = -1;
        for(int i = pos + chunkSize - 1; i >= pos; i--) {
          if(fullText.charAt(i) == ' ') {
            lastSpace = i;
            break;
          }
        }
        
        // If we found a space, break there
        if(lastSpace != -1 && lastSpace > pos) {
          chunkSize = lastSpace - pos;
        }
        // If no space found (word longer than 20 chars), just break at 20
      }
    }
    
    String line = fullText.substring(pos, pos + chunkSize);
    wrappedLines.push_back(line);
    
    pos += chunkSize;
    
    // Skip the space at the beginning of next line
    if(pos < textLen && fullText.charAt(pos) == ' ') {
      pos++;
    }
  }
}

void renderMessages() {
  if(!needsRedraw) return;
  
  lcd.clear();
  
  // Display 4 rows starting from currentRow
  for(int i = 0; i < 4; i++) {
    int lineIndex = currentRow + i;
    if(lineIndex < wrappedLines.size()) {
      lcd.setCursor(0, i);
      lcd.print(wrappedLines[lineIndex]);
    }
  }
  
  needsRedraw = false;
}

void handleScrollUp() {
  if(digitalRead(BTN1) == LOW && currentRow > 0) {
    currentRow--;
    needsRedraw = true;
    delay(200);
  }
}

void handleScrollDown() {
  if(digitalRead(BTN2) == LOW && currentRow < (int)wrappedLines.size() - 4) {
    currentRow++;
    needsRedraw = true;
    delay(200);
  }
}

bool handleExitMessages() {
  if(digitalRead(BTN4) == LOW) {
    delay(200);
    return true;
  }
  return false;
}

void handleNewMessage() {
  if(newMR) {
    buildFullText();
    wrapText();
    // Auto-scroll to bottom to show new message
    currentRow = max(0, (int)wrappedLines.size() - 4);
    needsRedraw = true;
    newMR = false;
  }
}

void initMessageDisplay() {
  if(messages.empty()) {
    lcd.clear();
    lcd.print("No messages yet");
    delay(2000);
    return;
  }
  
  buildFullText();
  wrapText();
  currentRow = max(0, (int)wrappedLines.size() - 4); // Start at bottom (latest messages)
  needsRedraw = true;
}

void displayMessages() {
  initMessageDisplay();
  if(messages.empty()) return;
  
  renderMessages();
  
  handleScrollUp();
  handleScrollDown();
  handleNewMessage();
  renderMessages();
}