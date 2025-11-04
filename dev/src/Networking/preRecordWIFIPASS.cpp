#include <Arduino.h>
#include <preRecordWIFIPASS.h>

static const ssid_entry_t ssid_table[] = {
    {"One Plus Nord 5", "aserfgbn"}};
static const size_t ssid_table_count = sizeof(ssid_table) / sizeof(ssid_table[0]);

bool lookup_password_for_ssid(const String &ssid, String &out_pass)
{
    if (ssid.length() == 0)
        return false;

    for (size_t i = 0; i < ssid_table_count; ++i)
    {
        if (ssid.equals(ssid_table[i].ssid))
        {
            out_pass = ssid_table[i].pass;
            return true;
        }
    }

    return false;
}

bool set_credentials_if_known(const String &ssid, String &pass)
{
    String pass_buffer;

    if (!lookup_password_for_ssid(ssid, pass_buffer))
    {
        return false;
    }

    pass = pass_buffer;

    return true;
}
