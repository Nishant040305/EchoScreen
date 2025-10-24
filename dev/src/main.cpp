#include <constants.h>
#include <LiquidCrystal.h>
#include <vector>
#include <WiFi.h>
#include <esp_now.h>
#include <showLandingScreen.h>
#include <handlerButtonMenu.h>
#include <handlButtonNetworkMenu.h>
#include <setupFindDevice.h>
#include <handleCodeInput.h>
#include<SocketIOEventHandler.h>
#include<SendRecieveHandler.h>
#include<sendConnectedUser.h>
#include<webSocketEvent.h>
LiquidCrystal lcd(19, 23, 18, 17, 16, 15);
String code = "";
void setup(){
  lcd.begin(20,4);
  Serial.begin(115200);
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);
  pinMode(BTN4, INPUT_PULLUP);
  showLandingScreen();
}
void loop(){
  if(socketConnected){
    socketIO.loop();
  }
  switch (status) {
    case 0: // Landing Screen
      showLandingScreen();
      while (handlerButtonMenu());
      break;
    case 1:
      setupFindDevice();
      break;
    case 3: // Network List
      displayNetworks();
      handleNetworkMenu();
      break;
    case 4:
      // status = 5;
      // break;
      if(!NetworkSSID){status = 1;}
      else{
        NetworkEntry entry = *NetworkSSID;
        Serial.printf("SSID: %s\n",entry.ssid.c_str());
        // Serial.printf("SSID: %s\n",entry.mac.c_str());
        lcd.clear();
        lcd.print("Press BTN1 for Pass");
        while(handleCodeInput(code,8));
        status=5;
        delay(1000);
      }
    case 5:
      if(!NetworkSSID){
        status=1;
      }
      else{
        NetworkEntry entry = *NetworkSSID;
        lcd.clear();
        lcd.print("Connecting...");
        lcd.setCursor(0,1);
        lcd.print(entry.ssid);
        
        WiFi.disconnect();
        delay(100);
        
        WiFi.begin(entry.ssid.c_str(), code.c_str());
        int attempts = 0;
        int maxAttempts = 20;
        while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
          delay(500);
          Serial.print(".");
          attempts++;
        }
        
        lcd.clear();
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("\nConnected!");
          Serial.print("IP: ");
          Serial.println(WiFi.localIP());
          lcd.print("Connected!");
          lcd.setCursor(0,1);
          lcd.print("IP:");
          lcd.print(WiFi.localIP());
          delay(3000);
          status = 6; 
        }
        else {
          Serial.println("\nConnection Failed!");
          lcd.print("Connection Failed!");
          lcd.setCursor(0,1);
          lcd.print("Wrong password?");
          delay(3000);
          code = "";
          status = 3;
        }
      }
      break;
      // case 6:{ // Socket.IO Connection
      //   lcd.clear();
      //   lcd.print("Connecting to");
      //   lcd.setCursor(0,1);
      //   lcd.print("server...");
        
      //   // Configure SSL certificate
      //   socketIO.beginSSL("esp-32-morse-based-chat-application.onrender.com", 443, "/socket.io/?EIO=4", "");
        
      //   // Or disable certificate verification (not recommended for production)
      //   // socketIO.beginSSL("esp-32-morse-based-chat-application.onrender.com", 443, "/socket.io/?EIO=4", "");
        
      //   socketIO.onEvent(socketIOEvent);
      //   socketIO.setReconnectInterval(5000);
        
      //   // Enable debug output
      //   // socketIO.enableHeartbeat(15000, 3000, 2);
        
      //   Serial.println("Waiting for Socket.IO connection...");
        
      //   // Wait for connection
      //   int wsAttempts = 0;
      //   while(!socketConnected && wsAttempts < 60) { // Increased timeout
      //     socketIO.loop();
      //     delay(500); // Increased delay
      //     if(wsAttempts % 5 == 0) {
      //       Serial.print(".");
      //     }
      //     wsAttempts++;
      //   }
        
      //   if(socketConnected) {
      //     Serial.println("\n✓ Socket.IO connected!");
          
      //     lcd.clear();
      //     lcd.print("Server Connected!");
      //     delay(1000);
          
      //     // Send connectUser event
      //     sendConnectUser();
      //     Serial.println("Sent connectUser, waiting for userId...");
          
      //     // Wait for userId response
      //     int userIdAttempts = 0;
      //     while(userId == "" && userIdAttempts < 60 && socketConnected) { // Check socketConnected
      //       socketIO.loop();
      //       delay(500); // Increased delay
      //       userIdAttempts++;
            
      //       if(userIdAttempts % 5 == 0) {
      //         Serial.print(".");
      //       }
      //     }
          
      //     if(userId != "") {
      //       Serial.println("\n✓ Successfully received userId: " + userId);
      //       // status will be set to 7 in socketIOEvent
      //     }
      //     else if(!socketConnected) {
      //       Serial.println("\n✗ Disconnected while waiting for userId");
      //       lcd.clear();
      //       lcd.print("Connection Lost");
      //       delay(2000);
      //       status = 0;
      //     }
      //     else {
      //       Serial.println("\n✗ Timeout waiting for userId");
      //       lcd.clear();
      //       lcd.print("No User ID");
      //       lcd.setCursor(0,1);
      //       lcd.print("Timeout");
      //       delay(2000);
      //       socketIO.disconnect();
      //       socketConnected = false;
      //       status = 0;
      //     }
      //   }
      //   else {
      //     Serial.println("\n✗ Socket.IO connection failed!");
      //     lcd.clear();
      //     lcd.print("Server Error");
      //     lcd.setCursor(0,1);
      //     lcd.print("Cannot connect");
      //     delay(3000);
      //     status = 0;
      //   }
      //   break;      }
    case 6:{ // WebSocket Connection
      lcd.clear();
      lcd.print("Connecting to");
      lcd.setCursor(0,1);
      lcd.print("server...");
      
      Serial.println("Initializing WebSocket connection...");
      
      // Connect to WebSocket endpoint
      // For Render.com with SSL
      webSocket.beginSSL("esp-32-morse-based-chat-application.onrender.com", 443, "/socket.io/?EIO=4&transport=websocket");
      
      // Set event handler
      webSocket.onEvent(webSocketEvent);
      
      // Set reconnect interval
      webSocket.setReconnectInterval(5000);
      
      // Disable ping/pong timeout (sometimes causes issues)
      webSocket.enableHeartbeat(15000, 3000, 2);
      
      Serial.println("Waiting for connection...");
      
      // Wait for connection
      unsigned long startTime = millis();
      unsigned long timeout = 30000; // 30 second timeout
      
      while(!socketConnected && (millis() - startTime) < timeout) {
        webSocket.loop();
        delay(100);
        
        if((millis() - startTime) % 1000 == 0) {
          Serial.print(".");
        }
      }
      
      if(socketConnected) {
        Serial.println("\n✓ WebSocket connected!");
        
        // Wait for userId
        startTime = millis();
        timeout = 20000; // 20 second timeout for userId
        
        while(userId == "" && socketConnected && (millis() - startTime) < timeout) {
          webSocket.loop();
          delay(100);
        }
        
        if(userId != "") {
          Serial.println("✓ Successfully received userId!");
          // status already set to 7 in webSocketEvent
        }
        else if(!socketConnected) {
          Serial.println("✗ Disconnected while waiting");
          lcd.clear();
          lcd.print("Connection Lost");
          delay(2000);
          status = 0;
        }
        else {
          Serial.println("✗ Timeout waiting for userId");
          lcd.clear();
          lcd.print("No User ID");
          delay(2000);
          webSocket.disconnect();
          status = 0;
        }
      }
      else {
        Serial.println("\n✗ WebSocket connection failed!");
        Serial.println("Check:");
        Serial.println("1. Server is running");
        Serial.println("2. URL is correct");
        Serial.println("3. SSL certificate");
        
        lcd.clear();
        lcd.print("Connection Failed");
        lcd.setCursor(0,1);
        lcd.print("Check server");
        delay(3000);
        status = 0;
      }
      break;
    
    }
    case 7:
      // Main connected state
      socketIO.loop(); // Keep connection alive
      
      lcd.clear();
      lcd.print("Connected");
      lcd.setCursor(0,1);
      if(userId != "") {
        lcd.print("ID: " + userId);
      } else {
        lcd.print("Waiting for ID...");
      }
      
      // Example: Send data to server
      // if(digitalRead(BTN1) == LOW) {
      //   sendSocketEvent("buttonPress", "BTN1");
      //   delay(500);
      // }
      
      delay(100);
      break;
      
    default:
      status = 0;
      break;
    }

  }
