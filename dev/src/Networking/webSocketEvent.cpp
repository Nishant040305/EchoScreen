#include<webSocketEvent.h>
#include<SendRecieveHandler.h>

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("[WS] Disconnected!");
      socketConnected = false;
      lcd.clear();
      lcd.print("Disconnected!");
      // Don't clear userId/roomId - we might reconnect
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
          delay(500);
          
          // Reconnection logic
          if(userId == "") {
            sendConnectUser();
          } else if(roomId != "" && status >= 10) {
            // Rejoin the room if we were in one
            Serial.println("Reconnecting to room: " + roomId);
            sendJoinRoom(roomId);
          }
        }
        else if(msg.startsWith("42")) {
          // Event message - parse JSON array
          String jsonStr = msg.substring(2); // Remove "42" prefix
          
          Serial.println("📥 Raw JSON: " + jsonStr);
          
          DynamicJsonDocument doc(2048);
          DeserializationError error = deserializeJson(doc, jsonStr);
          
          if(!error) {
            // Socket.IO sends events as arrays: ["eventName", data]
            if(doc.is<JsonArray>()) {
              JsonArray arr = doc.as<JsonArray>();
              String eventName = arr[0].as<String>();
              
              Serial.println("📨 Event: " + eventName);
              
              if(eventName == "userId") {
                userId = arr[1].as<String>();
                Serial.println("✅ Received userId: " + userId);
                
                lcd.clear();
                lcd.print("User ID:");
                lcd.setCursor(0, 1);
                lcd.print(userId.substring(0, 20));
                delay(2000);
                
                status = 7;
              }
              else if(eventName == "roomId") {
                roomId = arr[1].as<String>();
                isCreator = true;
                Serial.println("✅ Room created: " + roomId);
                
                lcd.clear();
                lcd.print("Room Created!");
                lcd.setCursor(0, 1);
                lcd.print("ID: " + roomId.substring(0, 16));
                delay(3000);
                
                status = 13;  // Go directly to chat
              }
              else if(eventName == "joinedRoom") {
                // Backend sends: {roomId: "xxx", userId: "yyy"}
                JsonObject data = arr[1].as<JsonObject>();
                String joinedRoomId = data["roomId"].as<String>();
                String joinedUserId = data["userId"].as<String>();
                
                Serial.println("✅ Joined room: " + joinedRoomId);
                Serial.println("   User: " + joinedUserId);
                
                roomId = joinedRoomId;
                
                lcd.clear();
                lcd.print("**ROOM JOINED**");
                lcd.setCursor(0, 1);
                lcd.print("ID: " + roomId.substring(0, 16));
                delay(3000);
                
                status = 13;  // Move to chat state
              }
              else if(eventName == "audioTranscription") {
                String transcription = arr[1].as<String>();
                Serial.println("📝 Transcription: " + transcription);
                
                Message newMsg;
                newMsg.type = "transcription";
                newMsg.text = transcription;
                newMsg.timestamp = millis();
                messages.push_back(newMsg);
                newMR = true;
                delay(300);
                
              }
              else if(eventName == "error") {
                String errorMsg = arr[1].as<String>();
                Serial.println("❌ Error: " + errorMsg);
                
                lcd.clear();
                lcd.print("Error:");
                lcd.setCursor(0, 1);
                lcd.print(errorMsg.substring(0, 20));
                delay(3000);
                
                // Go back to join/create menu
                roomId = "";
                status = 8;
              }
            }
          } else {
            Serial.println("❌ JSON Parse Error: " + String(error.c_str()));
          }
        }
        else if (msg == "2") {
          // Socket.IO PING
          webSocket.sendTXT("3");  // send PONG
          Serial.println("[WS] Responded with PONG (3)");
        }
      }
      break;
      
    case WStype_BIN:
      Serial.println("[WS] Binary data received");
      break;
      
    case WStype_ERROR:
      Serial.println("[WS] Error!");
      socketConnected = false;
      break;
      
    case WStype_PING:
      Serial.println("[WS] Ping");
      break;
      
    case WStype_PONG:
      Serial.println("[WS] Pong");
      break;
  }
}