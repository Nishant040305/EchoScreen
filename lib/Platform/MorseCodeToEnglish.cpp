/*
  Morse Code Editor for ESP32
  ---------------------------
  This code defines a function `editMorseString` that interprets Morse code input 
  character-by-character and builds a decoded English message.

  Features:
  - Accepts '.' and '-' as Morse code input
  - Uses '*' to submit the current Morse sequence (i.e., convert it to a character)
  - Adds space ' ' between words
  - Supports a manual override mode (if `functionality == true`) to directly insert characters

  Parameters:
    input            - The input character ('.', '-', '*', ' ', or direct character)
    functionality    - If true, bypass Morse logic and directly insert the character
    currentMorse     - (reference) The current Morse code being constructed
    decodedMessage   - (reference) The final message being built from decoded characters

  Morse map supports A–Z and 0–9.
*/

#include <map>

std::map<String, char> morseToChar = {
  {".-", 'A'},   {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'},  {".", 'E'},
  {"..-.", 'F'}, {"--.", 'G'},  {"....", 'H'}, {"..", 'I'},   {".---", 'J'},
  {"-.-", 'K'},  {".-..", 'L'}, {"--", 'M'},   {"-.", 'N'},   {"---", 'O'},
  {".--.", 'P'}, {"--.-", 'Q'}, {".-.", 'R'},  {"...", 'S'},  {"-", 'T'},
  {"..-", 'U'},  {"...-", 'V'}, {".--", 'W'},  {"-..-", 'X'}, {"-.--", 'Y'},
  {"--..", 'Z'}, {"-----", '0'}, {".----", '1'}, {"..---", '2'}, {"...--", '3'},
  {"....-", '4'}, {".....", '5'}, {"-....", '6'}, {"--...", '7'}, {"---..", '8'},
  {"----.", '9'}
};

void editMorseString(char input, bool functionality, String &currentMorse, String &decodedMessage) {
  if (functionality) {
    currentMorse = "";
    decodedMessage += input;
    return;
  }

  if (input == '.' || input == '-') {
    currentMorse += input;
    Serial.print("Current Morse: ");
    Serial.println(currentMorse);
  } else if (input == '\n' || input == '\r') {
    // Ignore newline from Serial
    return;
  } else if (input == '*') {  
    if (currentMorse.length() > 0) {
      if (morseToChar.count(currentMorse)) {
        char decodedChar = morseToChar[currentMorse];
        decodedMessage += decodedChar;
        Serial.print("Added: ");
        Serial.println(decodedChar);
      } else {
        Serial.print("Unknown Morse: ");
        Serial.println(currentMorse);
        decodedMessage += '?';
      }
      currentMorse = "";
    }
    Serial.print("Message: ");
    Serial.println(decodedMessage);
  } else if (input == ' ') {
    decodedMessage += ' ';
    Serial.print("Message: ");
    Serial.println(decodedMessage);
  }
}
