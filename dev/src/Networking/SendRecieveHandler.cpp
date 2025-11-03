#include <SendRecieveHandler.h>

// Create a new room
void sendCreateRoom()
{
  if (!socketConnected || userId == "")
  {
    Serial.println("Cannot create room: Not connected or no userId");
    return;
  }

  DynamicJsonDocument doc(256);
  JsonArray array = doc.to<JsonArray>();
  array.add("createRoom");
  array.add(userId);

  String output = "42"; // Socket.IO event message prefix
  serializeJson(array, output);

  webSocket.sendTXT(output);
  Serial.println("Sent createRoom: " + output);

  lcd.clear();
  lcd.print("Creating Room...");
}

void sendJoinRoom(String roomToJoin)
{
  if (!socketConnected || userId == "")
  {
    Serial.println("❌ Cannot join: Not ready");
    Serial.println("  socketConnected: " + String(socketConnected));
    Serial.println("  userId: " + userId);
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
  Serial.println("📤 Sent joinRoom: " + event);

  lcd.clear();
  lcd.print("Joining room...");
  lcd.setCursor(0, 1);
  lcd.print(roomToJoin);
}

// Send audio transcription
void sendAudioTranscription(String &audioFile)
{
  if (!socketConnected || roomId == "" || userId == "")
  {
    Serial.println("Cannot send audio: Not connected or not in a room");
    return;
  }

  DynamicJsonDocument doc(8192); // Larger buffer for audio data
  JsonArray array = doc.to<JsonArray>();
  array.add("audioTranscription");

  JsonObject data = array.createNestedObject();
  data["roomId"] = roomId;
  data["userId"] = userId;
  data["audioFile"] = audioFile; // Base64 encoded audio data

  String output = "42";
  serializeJson(array, output);

  webSocket.sendTXT(output);
  Serial.println("Sent audio transcription");

  lcd.clear();
  lcd.print("Sending Audio...");
  delay(1000);
}
