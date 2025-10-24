#include<constants.h>
#include<displayMessage.h>
void displayMessages() {
  if(messages.empty()) {
    lcd.clear();
    lcd.print("No messages yet");
    delay(2000);
    return;
  }
  
  int currentMsg = messages.size() - 1; // Start with latest
  
  while(true) {
    lcd.clear();
    lcd.print("Msg " + String(currentMsg + 1) + "/" + String(messages.size()));
    lcd.setCursor(0,1);
    
    String text = messages[currentMsg].text;
    // Display message across multiple lines
    lcd.print(text.substring(0, 20));
    lcd.setCursor(0,2);
    if(text.length() > 20) {
      lcd.print(text.substring(20, 40));
    }
    lcd.setCursor(0,3);
    if(text.length() > 40) {
      lcd.print(text.substring(40, 60));
    }
    
    delay(100);
    
    // BTN1: Previous message
    if(digitalRead(BTN1) == LOW && currentMsg > 0) {
      currentMsg--;
      delay(300);
    }
    
    // BTN2: Next message
    if(digitalRead(BTN2) == LOW && currentMsg < messages.size() - 1) {
      currentMsg++;
      delay(300);
    }
    
    // BTN4: Exit message view
    if(digitalRead(BTN4) == LOW) {
      delay(200);
      break;
    }
  }
}