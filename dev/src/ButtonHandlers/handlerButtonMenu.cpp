#include<Arduino.h>
#include<constants.h>
extern int status;
bool handlerButtonMenu(){
  if (digitalRead(BTN1) == LOW) {
    status = 1;
    delay(500);
    return false;
  }

  if (digitalRead(BTN2) == LOW) {
    status = 2;
    delay(500);
    return false;
  }

  if(digitalRead(BTN3)==LOW){
    status = 0;
    return false;
  }
  return true;
}
