#include "SocketIOEventHandler.h"
void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case sIOtype_DISCONNECT:
      Serial.println("[Socket.IO] Disconnected!");
      socketConnected = false;
      userId = "";
      roomId = "";
      lcd.clear();
      lcd.print("Disconnected!");
      break;
      
    case sIOtype_CONNECT:
      Serial.println("[Socket.IO] Connected to server!");
      socketConnected = true;
      
      lcd.clear();
      lcd.print("Connected!");
      lcd.setCursor(0,1);
      lcd.print("Server ready");
      
      // Don't send connectUser here - wait for explicit call
      break;
      
    case sIOtype_EVENT:
      {
        Serial.printf("[Socket.IO] Event: %s\n", payload);
        
        // Parse JSON event
        DynamicJsonDocument doc(10240);
        DeserializationError error = deserializeJson(doc, payload, length);
        
        if(error) {
          Serial.print("JSON parse error: ");
          Serial.println(error.c_str());
          return;
        }
        
        // Get event name
        String eventName = doc[0];
        
        // Handle "userId" event
        if(eventName == "userId") {
          userId = doc[1].as<String>();
          Serial.println(" Received userId: " + userId);
          
          lcd.clear();
          lcd.print("User ID Received:");
          lcd.setCursor(0,1);
          lcd.print(userId.substring(0, 20));
          delay(2000);
          
          status = 7; // Move to main menu
        }
        
        // Handle "roomId" event (when room is created)
        else if(eventName == "roomId") {
          roomId = doc[1].as<String>();
          isCreator = true;
          Serial.println("Room created: " + roomId);
          
          lcd.clear();
          lcd.print("Room Created!");
          lcd.setCursor(0,1);
          lcd.print("ID: " + roomId.substring(0, 16));
          delay(3000);
          
          status = 8; // Move to room state
        }
        
        // Handle "audioTranscription" event
        else if(eventName == "audioTranscription") {
          String transcription = doc[1].as<String>();
          Serial.println("Transcription: " + transcription);
          
          // Store message
          Message msg;
          msg.type = "transcription";
          msg.text = transcription;
          msg.timestamp = millis();
          messages.push_back(msg);
          
          // Display on LCD
          lcd.clear();
          lcd.print("New Message:");
          lcd.setCursor(0,1);
          lcd.print(transcription.substring(0, 20));
          lcd.setCursor(0,2);
          if(transcription.length() > 20) {
            lcd.print(transcription.substring(20, 40));
          }
          
          delay(3000);
        }
        
        // Handle "userJoined" event
        // else if(eventName == "userJoined") {
        //   String joinedUserId = doc[1].as<String>();
        //   Serial.println("👤 User joined: " + joinedUserId);
          
        //   lcd.clear();
        //   lcd.print("User Joined!");
        //   lcd.setCursor(0,1);
        //   lcd.print(joinedUserId.substring(0, 20));
        //   delay(2000);
        // }
        
        // Handle "error" event
        else if(eventName == "error") {
          String errorMsg = doc[1].as<String>();
          Serial.println("❌ Error: " + errorMsg);
          
          lcd.clear();
          lcd.print("Error:");
          lcd.setCursor(0,1);
          lcd.print(errorMsg);
          delay(3000);
        }
      }
      break;
      
    case sIOtype_ACK:
      Serial.printf("[Socket.IO] ACK: %u\n", length);
      break;
      
    case sIOtype_ERROR:
      Serial.printf("[Socket.IO] Error: %s\n", payload);
      lcd.clear();
      lcd.print("Socket Error!");
      break;
      
    case sIOtype_BINARY_EVENT:
    case sIOtype_BINARY_ACK:
      Serial.println("[Socket.IO] Binary data received");
      break;
  }
}