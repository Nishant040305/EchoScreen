#ifndef CONSTANTS_H
#define CONSTANTS_H

/*
B1 - Button 1
This button is connected to GPIO 12.
Pressing this button adds '.' character into the buffer.

B2 - Button 2
This button is connected to GPIO 13.
Pressing this button adds '_' character into the buffer.

B3 - Button 3
This button is connected to GPIO 14.

Scenario1: Morse Buffer is not empty
Pressing this button converts the buffer into the equivalent character ands to the message.
and empty the buffer.

Scenario2: Morse Buffer is empty
Pressing this button adds a space ' ' character into the message.
and buffer remains empty.

B4 - Button 4
This button is connected to GPIO 27.
Pressing this button clears both the buffer and the message.

*/
// Pin definitions
const int B1 = 12;
const int B2 = 13;
const int B3 = 14;
const int B4 = 27;
#endif
