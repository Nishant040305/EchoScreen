#include<webSocketEvent.h>
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("[WS] Disconnected!");
      socketConnected = false;
      userId = "";
      lcd.clear();
      lcd.print("Disconnected!");
      break;
      
    case WStype_CONNECTED:
      {
        Serial.println("[WS] Connected!");
        socketConnected = true;
        
        // Socket.IO handshake - send "40" to connect to default namespace
        webSocket.sendTXT("40");
        Serial.println("Sent Socket.IO connect (40)");
      }
      break;
      
    case WStype_TEXT:
      {
        String msg = String((char*)payload);
        Serial.printf("[WS] Message: %s\n", msg.c_str());
        
        // Socket.IO protocol parsing
        if(msg.startsWith("0")) {
          // Connection handshake response
          Serial.println("Socket.IO handshake received");
        }
        else if(msg.startsWith("40")) {
          // Connected to namespace
          Serial.println("Connected to Socket.IO namespace");
          
          lcd.clear();
          lcd.print("Connected!");
          lcd.setCursor(0,1);
          lcd.print("Sending request...");
          
          // Send connectUser event
          delay(500);
          sendConnectUser();
        }
        else if(msg.startsWith("42")) {
          // Event message - parse JSON
          String jsonStr = msg.substring(2); // Remove "42" prefix
          
          DynamicJsonDocument doc(1024);
          DeserializationError error = deserializeJson(doc, jsonStr);
          
          if(!error) {
            String eventName = doc[0];
            
            if(eventName == "userId") {
              userId = doc[1].as<String>();
              Serial.println("✓ Received userId: " + userId);
              
              lcd.clear();
              lcd.print("User ID:");
              lcd.setCursor(0,1);
              lcd.print(userId.substring(0, 20));
              delay(2000);
              
              status = 7;
            }
            else if(eventName == "roomId") {
              roomId = doc[1].as<String>();
              isCreator = true;
              Serial.println("✓ Room created: " + roomId);
              
              lcd.clear();
              lcd.print("Room Created!");
              lcd.setCursor(0,1);
              lcd.print("ID: " + roomId.substring(0, 16));
              delay(3000);
              
              status = 8;
            }
            else if(eventName == "audioTranscription") {
              String transcription = doc[1].as<String>();
              Serial.println("📝 Transcription: " + transcription);
              
              Message newMsg;
              newMsg.type = "transcription";
              newMsg.text = transcription;
              newMsg.timestamp = millis();
              messages.push_back(newMsg);
              
              lcd.clear();
              lcd.print("New Message:");
              lcd.setCursor(0,1);
              lcd.print(transcription.substring(0, 20));
              delay(3000);
            }
          }
        }
      }
      break;
      
    case WStype_BIN:
      Serial.println("[WS] Binary data received");
      break;
      
    case WStype_ERROR:
      Serial.println("[WS] Error!");
      break;
      
    case WStype_PING:
      Serial.println("[WS] Ping");
      break;
      
    case WStype_PONG:
      Serial.println("[WS] Pong");
      break;
  }
}