#include<Arduino.h>
#include<constants.h>
#include<setupFindDevice.h>
void handleNetworkMenu() {
    static int lastSelectedIndex = -1;  // track last LCD display
    static unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 150; // ms

    int b1 = digitalRead(BTN1); // Scroll up
    int b2 = digitalRead(BTN2); // Scroll down
    int b3 = digitalRead(BTN3); // Action button
    int b4 = digitalRead(BTN4); // Action button

    unsigned long currentMillis = millis();

    // Scroll down
    if (b2 == LOW && (currentMillis - lastDebounceTime > debounceDelay)) {
        if (selectedIndex < (int)networkList.size() - 1) {
            selectedIndex++;
            if (selectedIndex >= displayStart + 3) {
                displayStart++;
            }
        }
        lastDebounceTime = currentMillis;
    }

    // Scroll up
    if (b1 == LOW && (currentMillis - lastDebounceTime > debounceDelay)) {
        if (selectedIndex > 0) {
            selectedIndex--;
            if (selectedIndex < displayStart) {
                displayStart--;
            }
        }
        lastDebounceTime = currentMillis;
    }

    // Update LCD only if selection changed
    if (lastSelectedIndex != selectedIndex) {
        displayNetworks();
        lastSelectedIndex = selectedIndex;
    }

    // Action buttons
    if (b3 == LOW) {
        status = 1;  // example: go back / exit
    }

    if (b4 == LOW) {
        if (!networkList.empty()) {
            NetworkSSID = &networkList[selectedIndex]; // select network
            status = 4;  // move to next menu / connect
        } else {
            status = 1;  // fallback
        }
    }
}
