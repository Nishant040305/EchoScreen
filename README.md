# EchoScreen

EchoScreen is a multi-component system for real-time audio capture, visualization, and transcription. It consists of:

- Node.js backend (Socket.IO + HTTP + Redis) for room/user coordination and transcription requests.
- ESP32 firmware (PlatformIO/Arduino) for device UI, network handling, and audio streaming.
- Java desktop WebSocket server with a live waveform UI and optional auto-recording/transcription bridge.

## Repository Structure

```
EchoScreen/
├── backend/          # Node.js server
│   ├── src/
│   │   ├── server.js       # Express + Socket.IO entry
│   │   ├── events/         # Socket.IO event handlers
│   │   ├── config/redisClient.js
│   │   ├── globals.js      # Redis-backed room/user helpers
│   │   └── index.html      # Landing page
│   ├── app.js              # Thin entry that runs src/server
│   ├── .env.example        # Environment variables template
│   └── package.json
├── dev/              # ESP32 firmware (PlatformIO)
│   ├── platformio.ini      # Board, framework, serial config
│   ├── include/            # Headers (pins, WiFi map, websocket, etc.)
│   └── src/                # Firmware sources
└── java_server/      # Java desktop WebSocket server + UI
    ├── AudioWebSocketServerWithPlot.java
    ├── run.bat             # Compile & run helper (Windows)
    └── *.jar               # Required dependencies
```

## Prerequisites

- Backend
  - `Node.js` 18+ and `npm`.
  - `Redis` server (local or managed). Default URL `redis://localhost:6379`.
  - AssemblyAI API key for transcription.

- ESP32 Firmware (dev)
  - `VS Code` + `PlatformIO` extension or PlatformIO Core CLI.
  - Board: `esp32doit-devkit-v1`.

- Java Desktop Server
  - `Java JDK` 8+.
  - Windows recommended (provided `run.bat`), but can run on any OS with equivalent commands.

## Quick Start

- Backend: install and run
  1. Open `backend/`.
  2. Copy `.env.example` to `.env` and set values (see below).
  3. Install deps: `npm install`.
  4. Development: `npm run dev` (nodemon). Production: `npm start`.
  5. Server listens on port `4000` by default.

- ESP32 firmware: build and flash
  1. Open `dev/` in VS Code with PlatformIO.
  2. Adjust serial ports in `platformio.ini` (`upload_port`, `monitor_port`).
  3. Set backend URL in `dev/src/constants.cpp` (see below).
  4. Build: `pio run`. Upload: `pio run -t upload`. Monitor: `pio device monitor`.

- Java desktop server: compile and run
  1. Open `java_server/`.
  2. Double-click `run.bat` or run from terminal to compile and start.
  3. Server binds to port `8081` and opens the waveform UI.

## Backend Setup

- Environment (`backend/.env`)

```
REDIS_URL=redis://localhost:6379
ASSEMBLYAI_API_KEY=your-api-key
PORT=4000   # optional; defaults to 4000
```

- Scripts (`backend/package.json`)
  - `npm run dev`: runs `src/server.js` via nodemon.
  - `npm start`: runs `src/server.js`.

- Behavior
  - Express serves `index.html` on `/`.
  - Socket.IO is enabled with CORS `origin: *`.
  - Redis is used to store rooms/users via `src/config/redisClient.js` and `src/globals.js`.

## ESP32 Firmware (dev)

- Board & serial config (`dev/platformio.ini`)

```
[env:esp32doit-devkit-v1]
platform = espressif32
board = esp32doit-devkit-v1
framework = arduino
upload_port = COM3
upload_speed = 115200
monitor_port = COM3
monitor_speed = 115200
```

- Backend URL
  - Update the backend host in `dev/src/constants.cpp`:

```
String URL = "echoscreen.onrender.com"; // change to your backend host
```

  - If self-hosting locally, use your machine’s IP or hostname, e.g. `"192.168.1.10:4000"`.
  - For HTTPS deployments, prefer secure WebSockets (`wss://`) and HTTPS URLs.

- WiFi credentials
  - WiFi SSID/password helpers live in `dev/include/ssid_pass_map.h`.
  - Add or implement lookup/set functions to prefill known networks.

- Pins & peripherals
  - Pins for buttons, LCD, and I2S microphone are defined in `dev/include/constants.h`.

## Java Desktop WebSocket Server

- Purpose
  - Accepts raw PCM audio via WebSocket from clients (e.g., ESP32), renders a live waveform, and optionally auto-records 5-second clips for transcription.

- How to run (Windows)
  - In `java_server/`, run `run.bat`. It compiles and starts:

```
javac -cp ".;Java-WebSocket-1.5.4.jar;slf4j-api-2.0.9.jar;concentus-1.0.1.jar;slf4j-simple-2.0.9.jar;socket.io-client-2.1.0.jar;engine.io-client-2.1.0.jar;json-20180813.jar;okhttp-3.12.12.jar;okio-1.17.5.jar" AudioWebSocketServerWithPlot.java
java -cp ".;Java-WebSocket-1.5.4.jar;slf4j-api-2.0.9.jar;concentus-1.0.1.jar;slf4j-api-2.0.9.jar;slf4j-simple-2.0.9.jar;socket.io-client-2.1.0.jar;engine.io-client-2.1.0.jar;json-20180813.jar;okhttp-3.12.12.jar;okio-1.17.5.jar" AudioWebSocketServerWithPlot
```

- Defaults
  - WebSocket server port: `8081`.
  - Transcription endpoint: `https://echoscreen.onrender.com` (configured in code as `TRANSCRIPTION_SERVER`).

- Connecting a client
  - Point your client (ESP32 or other) to `ws://<PC-IP>:8081`.
  - The UI opens automatically and updates the waveform in real-time.

## Component Interaction

- ESP32 device discovers WiFi, connects, and communicates via WebSockets (`WebSocketsClient`) to the backend URL.
- Backend coordinates users/rooms with Redis and Socket.IO and serves a simple index page.
- Java desktop server (optional) can accept raw audio streams for local visualization and auto-recording; it can forward recordings for transcription.

## Deployment Notes

- Hosted backend
  - If deploying to Render or similar, set `URL` to your public domain (e.g., `echoscreen.onrender.com`).
  - Ensure CORS and Socket.IO transports (`websocket`, `polling`) are allowed.

- TLS
  - When hosting behind HTTPS, use `wss://` for WebSocket endpoints and configure any reverse proxy to forward `Upgrade` and `Connection` headers correctly.

## Troubleshooting

- Redis connection errors
  - Verify Redis is accessible at `REDIS_URL`. Check logs for `Redis Client Error`.

- Port conflicts
  - Backend default: `4000`. Java server default: `8081`. Change via env or code if needed.

- PlatformIO upload/monitor
  - Update `COM` ports in `platformio.ini`. Close other serial monitors before opening.

- Java compile/runtime
  - Ensure JDK is installed and `javac`/`java` are on `PATH`. On non-Windows systems, replicate the classpath from `run.bat`.

## License

See `LICENSE` in the repository root.