#include<SendRecieveHandler.h>
void sendSocketEvent(String eventName, String data) {
  if(socketConnected) {
    DynamicJsonDocument doc(10240);
    JsonArray array = doc.to<JsonArray>();
    
    array.add(eventName);
    array.add(data);
    
    String output;
    serializeJson(doc, output);
    
    socketIO.sendEVENT(output);
    Serial.println("Sent: " + output);
  }
}

void sendSocketEventWithJson(String eventName, JsonObject data) {
  if(socketConnected) {
    DynamicJsonDocument doc(10240);
    JsonArray array = doc.to<JsonArray>();
    
    array.add(eventName);
    array.add(data);
    
    String output;
    serializeJson(doc, output);
    
    socketIO.sendEVENT(output);
    Serial.println("Sent: " + output);
  }
}