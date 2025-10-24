#ifndef SETUP_FIND_DEVICE_H
#define SETUP_FIND_DEVICE_H

#include <Arduino.h>
#include <LiquidCrystal.h>
#include <vector>
#include <WiFi.h>
#include "constants.h"

// Struct representing a detected WiFi network
typedef struct NetworkEntry {
  String ssid;
  String mac;
} NetworkEntry;

// External variables defined in setupFindDevice.cpp
extern LiquidCrystal lcd;
extern std::vector<NetworkEntry> networkList;
extern int selectedIndex;
extern int displayStart;
extern int status;

// Function declarations
void setupFindDevice();
void displayNetworks();
void handleScrollButtons();

#endif  // SETUP_FIND_DEVICE_H
