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
#include <AudioHandler.h>
#include <preRecordWIFIPASS.h>
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
String code = "";
bool isRoomCreator = false; // Track if user created the room

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

    // Process audio chunks if recording
    if (audioHandler.getIsRecording())
    {
      audioHandler.processAudioChunk();
    }
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
    if (!NetworkSSID)
    {
      status = 1;
    }
    else
    {
      NetworkEntry entry = *NetworkSSID;
      Serial.printf("SSID: %s\n", entry.ssid.c_str());
      lcd.clear();
      if (!set_credentials_if_known(entry.ssid, code))
      {
        lcd.print("Press BTN1 for Pass");
        codeInputinit(false);
        while (handleCodeInput(code, 8))
          ;
      }
      status = 5;
      delay(1000);
    }
    break;

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
    IPAddress serverIP(10, 79, 73, 187);
    webSocket.begin(serverIP, 4000, "/socket.io/?EIO=4&transport=websocket");

    // webSocket.beginSSL(URL, 443, "/socket.io/?EIO=4&transport=websocket");
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);
    webSocket.enableHeartbeat(15000, 3000, 2);

    Serial.println("Waiting for connection...");

    unsigned long startTime = millis();
    unsigned long timeout = 30000;

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

      startTime = millis();
      timeout = 20000;

      while (userId == "" && socketConnected && (millis() - startTime) < timeout)
      {
        webSocket.loop();
        delay(100);
      }

      if (userId != "")
      {
        Serial.println(" Successfully received userId!");
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
    isRoomCreator = true; // Mark as room creator
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
      roomId = room;
      sendJoinRoom(room);
      isRoomCreator = false; // Not the creator
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
      status = 8;
    }
    break;
  }

  case 12:
  { // Waiting for room creation
    unsigned long startTime = millis();

    while (status == 12 && (millis() - startTime) < 15000)
    {
      webSocket.loop();
      delay(100);
    }

    if (status == 12)
    {
      Serial.println("Room creation timeout");
      lcd.clear();
      lcd.print("Timeout!");
      lcd.setCursor(0, 1);
      lcd.print("Try again");
      delay(2000);
      isRoomCreator = false;
      status = 8;
    }
    break;
  }

  case 13:
  {
    // Initialize audio when entering room
    if (!audioHandler.init())
    {
      Serial.println("Failed to initialize audio");
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Room: " + roomId.substring(0, 16));
    lcd.setCursor(0, 1);

    // Different options based on creator status
    if (isRoomCreator)
    {
      lcd.print("BTN1:Rec BTN3:Msgs");
    }
    else
    {
      lcd.print("BTN3: View messages");
    }

    bool viewingMessages = false;
    unsigned long lastButtonCheck = 0;
    const unsigned long buttonDebounce = 300;

    while (true)
    {
      webSocket.loop();

      // Process audio if recording
      if (audioHandler.getIsRecording())
      {
        audioHandler.processAudioChunk();
      }
      if (!viewingMessages)
      {
        unsigned long currentTime = millis();
        // BTN1: Toggle recording (only for creator)
        if (isRoomCreator && digitalRead(BTN1) == LOW &&
            (currentTime - lastButtonCheck) > buttonDebounce)
        {
          lastButtonCheck = currentTime;

          if (!audioHandler.getIsRecording())
          {
            audioHandler.startRecording();
            lcd.setCursor(0, 2);
            lcd.print("Recording...     ");
            Serial.println("Started recording");
          }
          else
          {
            audioHandler.stopRecording();
            lcd.setCursor(0, 2);
            lcd.print("                    ");
            Serial.println("Stopped recording");
          }
          delay(300);
        }

        // BTN3: Enter message view
        if (digitalRead(BTN3) == LOW &&
            (currentTime - lastButtonCheck) > buttonDebounce)
        {
          lastButtonCheck = currentTime;
          viewingMessages = true;
          initMessageDisplay();
          delay(300);
        }

        // BTN4: Exit room
        if (digitalRead(BTN4) == LOW &&
            (currentTime - lastButtonCheck) > buttonDebounce)
        {
          lastButtonCheck = currentTime;

          // Stop recording if active
          if (audioHandler.getIsRecording())
          {
            audioHandler.stopRecording();
          }

          // Cleanup audio
          audioHandler.cleanup();

          isRoomCreator = false;
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

          if (isRoomCreator)
          {
            lcd.print("BTN1:Rec BTN3:Msgs");
          }
          else
          {
            lcd.print("BTN3: View messages");
          }

          lcd.setCursor(0, 3);
          lcd.print("BTN4: Exit Room");
        }
      }

      delay(50);
    }
    break;
  }
  }
}