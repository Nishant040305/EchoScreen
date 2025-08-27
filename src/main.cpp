#include <WiFi.h>

const char* ssid = "Blaze Brij mohan srivastava-5G";
const char* pass = "110806ac";

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();   // listen for incoming clients
  if (client) {
    Serial.println("New Client connected.");
    String request = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;

        // End of request
        if (c == '\n') {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println("Connection: close");
          client.println();
          client.println("<!DOCTYPE html><html>");
          client.println("<h1>Hello from ESP32!</h1>");
          client.println("</html>");
          break;
        }
      }
    }
    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
}
