#include<showJoinCreateOption.h>

void showJoinCreateOption() {
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("Welcome to");
    lcd.setCursor(2, 1);
    lcd.print("** EchoScreen **");
    lcd.setCursor(0, 2);
    lcd.print("B1: Create  B2: Join");
}
  
void showJoin(){
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print(">> Press B1 <<");
    lcd.setCursor(2, 1);
    lcd.print("To Join a Room");
}