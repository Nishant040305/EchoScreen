#include <Arduino.h>
#include <LiquidCrystal.h>
extern LiquidCrystal lcd;  

void showLandingScreen() {
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("************");
    lcd.setCursor(4, 1);
    lcd.print("*EchoScreen*");
    lcd.setCursor(4, 2);
    lcd.print("************");
    lcd.setCursor(1, 3);
    lcd.print("Press BTN1 to Start");
}
  