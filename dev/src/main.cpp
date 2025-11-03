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
#include <SendRecieveHandler.h>
#include <sendConnectedUser.h>
#include <webSocketEvent.h>
#include <showJoinCreateOption.h>
#include <displayMessage.h>

LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
// LiquidCrystal lcd(18, 19, 21, 16, 17, 5);
String code = "";
void setup()
{
  lcd.begin(20, 4);
  Serial.begin(115200);

  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);
  pinMode(BTN4, INPUT_PULLUP);
  showLandingScreen();
}
void loop()
{
  if (socketConnected)
  {
    webSocket.loop();
  }
  switch (status)
  {
  case 0: // Landing Screen
    showLandingScreen();
    while (handlerButtonMenu())
      ;
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
    if (!NetworkSSID)
    {
      status = 1;
    }
    else
    {

      NetworkEntry entry = *NetworkSSID;
      Serial.printf("SSID: %s\n", entry.ssid.c_str());
      // Serial.printf("SSID: %s\n",entry.mac.c_str());
      lcd.clear();
      lcd.print("Press BTN1 for Pass");
      // while(handleCodeInput(code,8,false));
      codeInputinit(false);
      while (handleCodeInput(code, 8))
        ;
      status = 5;
      delay(1000);
    }
  case 5:
    if (!NetworkSSID)
    {
      status = 1;
    }
    else
    {
      NetworkEntry entry = *NetworkSSID;
      lcd.clear();
      lcd.print("Connecting...");
      lcd.setCursor(0, 1);
      lcd.print(entry.ssid);

      WiFi.disconnect();
      delay(100);

      WiFi.begin(entry.ssid.c_str(), code.c_str());
      int attempts = 0;
      int maxAttempts = 20;
      while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts)
      {
        delay(500);
        Serial.print(".");
        attempts++;
      }

      lcd.clear();
      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.println("\nConnected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        lcd.print("Connected!");
        lcd.setCursor(0, 1);
        lcd.print("IP:");
        lcd.print(WiFi.localIP());
        delay(3000);
        status = 6;
      }
      else
      {
        Serial.println("\nConnection Failed!");
        lcd.print("Connection Failed!");
        lcd.setCursor(0, 1);
        lcd.print("Wrong password?");
        delay(3000);
        code = "";
        status = 3;
      }
    }
    break;
  case 6:
  { // WebSocket Connection
    lcd.clear();
    lcd.print("Connecting to");
    lcd.setCursor(0, 1);
    lcd.print("server...");

    Serial.println("Initializing WebSocket connection...");

    // Connect to WebSocket endpoint
    // For Render.com with SSL
    webSocket.beginSSL(URL, 443, "/socket.io/?EIO=4&transport=websocket");

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

    while (!socketConnected && (millis() - startTime) < timeout)
    {
      webSocket.loop();
      delay(100);

      if ((millis() - startTime) % 1000 == 0)
      {
        Serial.print(".");
      }
    }

    if (socketConnected)
    {
      Serial.println("\nWebSocket connected!");

      // Wait for userId
      startTime = millis();
      timeout = 20000; // 20 second timeout for userId

      while (userId == "" && socketConnected && (millis() - startTime) < timeout)
      {
        webSocket.loop();
        delay(100);
      }

      if (userId != "")
      {
        Serial.println(" Successfully received userId!");
        // status already set to 7 in webSocketEvent
      }
      else if (!socketConnected)
      {
        Serial.println(" Disconnected while waiting");
        lcd.clear();
        lcd.print("Connection Lost");
        delay(2000);
        status = 0;
      }
      else
      {
        Serial.println("Timeout waiting for userId");
        lcd.clear();
        lcd.print("No User ID");
        delay(2000);
        webSocket.disconnect();
        status = 0;
      }
    }
    else
    {
      Serial.println("\n WebSocket connection failed!");
      Serial.println("Check:");
      Serial.println("1. Server is running");
      Serial.println("2. URL is correct");
      Serial.println("3. SSL certificate");

      lcd.clear();
      lcd.print("Connection Failed");
      lcd.setCursor(0, 1);
      lcd.print("Check server");
      delay(3000);
      status = 0;
    }
    break;
  }
  case 7:
    webSocket.loop();
    lcd.clear();
    lcd.print("Connected");
    lcd.setCursor(0, 1);
    if (userId != "")
    {
      lcd.print("ID: " + userId);
      delay(200);
      status = 8;
    }
    else
    {
      lcd.print("Waiting for ID...");
    }
    delay(500);
    break;
  case 8:
  {
    webSocket.loop();
    showJoinCreateOption();
    bool b = handleJoinCreateOption();
    while (b)
    {
      webSocket.loop();
      delay(50);
      b = handleJoinCreateOption();
    }
    break;
  }

  case 9: // Create Room
    webSocket.loop();
    createRoom();
    lcd.clear();
    lcd.print("Creating Room...");
    status = 12;
    break;

  case 10:
  { // Join Room
    String room = "";
    showJoin();
    codeInputinit(true);
    bool b = handleCodeInput(room, 4);
    while (b)
    {
      webSocket.loop();
      delay(50);
      b = handleCodeInput(room, 4);
    }

    if (room.length() > 0)
    {
      roomId = room; // Set roomId first
      sendJoinRoom(room);
      status = 11;
    }
    else
    {
      lcd.clear();
      lcd.print("Invalid Room ID");
      delay(2000);
      status = 8;
    }
    break;
  }

  case 11:
  { // Waiting for join confirmation
    unsigned long startTime = millis();
    lcd.clear();
    lcd.print("Joining...");
    lcd.setCursor(0, 1);
    lcd.print("Room: " + roomId.substring(0, 16));

    while (status == 11 && (millis() - startTime) < 15000)
    {
      webSocket.loop();
      delay(100);
    }

    if (status == 11)
    {
      Serial.println("Join confirmation timeout");
      lcd.clear();
      lcd.print("Room not found!");
      lcd.setCursor(0, 1);
      lcd.print("Try again");
      delay(3000);
      roomId = "";
      status = 8; // Go back to menu
    }
    break;
  }

  case 12:
  { // Waiting for room creation
    unsigned long startTime = millis();

    // Wait up to 15 seconds for roomId
    while (status == 12 && (millis() - startTime) < 15000)
    {
      webSocket.loop();
      delay(100);
    }

    // Timeout
    if (status == 12)
    {
      Serial.println("Room creation timeout");
      lcd.clear();
      lcd.print("Timeout!");
      lcd.setCursor(0, 1);
      lcd.print("Try again");
      delay(2000);
      status = 8; // Go back to menu
    }
    break;
  }

  case 13:
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Room: " + roomId.substring(0, 16));
    lcd.setCursor(0, 1);
    lcd.print("BTN3: View messages");

    bool viewingMessages = false;

    while (true)
    {
      webSocket.loop();

      if (!viewingMessages)
      {
        // BTN3: Enter message view
        if (digitalRead(BTN3) == LOW)
        {
          viewingMessages = true;
          initMessageDisplay();
          delay(300);
        }

        // BTN4: Exit room
        if (digitalRead(BTN4) == LOW)
        {
          status = 8;
          delay(200);
          break;
        }
      }
      else
      {
        handleScrollUp();
        handleScrollDown();
        handleNewMessage();
        renderMessages();
        if (handleExitMessages())
        {
          viewingMessages = false;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Room: " + roomId.substring(0, 16));
          lcd.setCursor(0, 1);
          lcd.print("BTN3: View messages");
          lcd.print("BTN4: Exit Room");
        }
      }

      delay(50);
    }
    break;
  }
  }
}