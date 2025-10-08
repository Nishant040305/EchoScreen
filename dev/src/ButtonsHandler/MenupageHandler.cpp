
void MenupageHandler(uint8_t B1,uint8_t B2,uint8_t B4){
    if (digitalRead(B1) == LOW) {
    setupSearchMode();
    delay(500);
  }

  if (digitalRead(B2) == LOW) {
    setupFindDevice();
    delay(500);
  }

  if (digitalRead(B4) == LOW) {
    showMenu();
    delay(500);
  }
}