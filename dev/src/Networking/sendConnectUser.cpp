#include<sendConnectedUser.h>
#include<constants.h>
#include<LiquidCrystal.h>
extern LiquidCrystal lcd;
// void sendConnectUser() {
//   if(!socketConnected) {
//     Serial.println("Cannot send connectUser - not connected");
//     return;
//   }
  
//   DynamicJsonDocument doc(1024);
//   JsonArray array = doc.to<JsonArray>();
//   array.add("connectUser");
  
//   String output;
//   serializeJson(doc, output);
//   socketIO.sendEVENT(output);
  
//   Serial.println("Sent: connectUser");
  
//   lcd.clear();
//   lcd.print("Requesting User ID");
// }
void sendConnectUser() {
  if(!socketConnected) {
    Serial.println("❌ Cannot send - not connected");
    return;
  }
  
  // Socket.IO event format: 42["eventName", data]
  String event = "42[\"connectUser\"]";
  webSocket.sendTXT(event);
  Serial.println("📤 Sent: connectUser");
}

void createRoom() {
  if(!socketConnected || userId == "") {
    Serial.println("Not ready to create room");
    return;
  }
  
  // Format: 42["createRoom", "userId"]
  String event = "42[\"createRoom\",\"" + userId + "\"]";
  webSocket.sendTXT(event);
  Serial.println("📤 Sent: createRoom");
  
  lcd.clear();
  lcd.print("Creating room...");
}

void joinRoom(String roomToJoin) {
  if(!socketConnected || userId == "") {
    Serial.println("Not ready to join room");
    return;
  }
  
  // Format: 42["joinRoom", {"roomId":"xxx", "userId":"yyy"}]
  DynamicJsonDocument doc(512);
  JsonObject data = doc.to<JsonObject>();
  data["roomId"] = roomToJoin;
  data["userId"] = userId;
  
  String jsonData;
  serializeJson(data, jsonData);
  
  String event = "42[\"joinRoom\"," + jsonData + "]";
  webSocket.sendTXT(event);
  Serial.println("📤 Sent: joinRoom - " + event);
  
  roomId = roomToJoin;
  isCreator = false;
  
  lcd.clear();
  lcd.print("Joining room...");
  lcd.setCursor(0,1);
  lcd.print(roomToJoin);
  
  status = 8;
}