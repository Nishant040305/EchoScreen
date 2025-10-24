#ifndef SETUP_FIND_DEVICE_H
#define SETUP_FIND_DEVICE_H

#include <Arduino.h>
#include <LiquidCrystal.h>
#include <vector>
#include <WiFi.h>
#include <constants.h>

extern LiquidCrystal lcd;
// Function declarations
void setupFindDevice();
void displayNetworks();
// void handleScrollButtons();

#endif  // SETUP_FIND_DEVICE_H
