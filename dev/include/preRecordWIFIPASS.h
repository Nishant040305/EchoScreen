#ifndef SSID_PASS_MAP_H
#define SSID_PASS_MAP_H

#include <Arduino.h>

#define MAX_SSID_LEN 32
#define MAX_PASS_LEN 64

typedef struct
{
    String ssid;
    String pass;
} ssid_entry_t;

bool lookup_password_for_ssid(const String &ssid, String &out_pass);
bool set_credentials_if_known(const String &ssid, String &pass);

#endif
