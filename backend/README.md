# EchoScreen Application Node.js Backend

## Features

- **Express Server**: Handles HTTP requests for room creation and joining
- **Socket.IO**: Manages real-time communication between clients
- **Session Management**: Tracks user sessions and handles reconnections
- **Room Service**: Manages room creation and access
- **Audio File Handling**: Accepts and processes audio files via HTTP endpoint and Socket.IO
- **AssemblyAI Integration**: Provides real-time audio transcription with speaker diarization
- **Creator-Controlled Audio**: Only room creators can send audio for transcription

## Project Structure

The backend is organized using a modular architecture:

```
backend2/
├── server.js                # Entry point
├── package.json             # Dependencies
├── .env                     # Environment variables
├── uploads/                 # Uploaded audio files (created at runtime)
├── test/                    # Test client
│   └── test.html            # Test interface
└── src/                     # Source code
    ├── server.js            # Main server setup
    ├── controllers/         # Request handlers
    │   ├── audioController.js
    │   ├── roomController.js
    │   ├── sessionController.js
    │   └── socketController.js
    ├── services/            # Business logic
    │   ├── audioService.js
    │   ├── roomService.js
    │   └── sessionManager.js
    ├── middleware/          # Custom middleware
    │   └── cookie-parser.js
    └── routes/              # API routes
        └── index.js
```

## Installation

1. Clone the repository
2. Navigate to the backend2 directory
3. Install dependencies:

```bash
npm install
```

4. Create a `.env` file with the following variables (or use the provided one):

```
PORT=5000
CORS_ORIGIN=*
SESSION_COOKIE_MAX_AGE=2592000000
SOCKET_PING_TIMEOUT=60000
SOCKET_PING_INTERVAL=25000
UPLOAD_DIR=uploads
MAX_FILE_SIZE=10485760
```

5. Start the server:

```bash
npm start
```

Or for development with auto-reload:

```bash
npm run dev
```

## API Endpoints

### HTTP Routes

- **POST /create_room**: Create a new chat room
- **GET /join_room**: Check if a room exists and join it
- **GET /session**: Get or create a session ID
- **POST /upload_audio**: Upload an audio file for processing

### Socket.IO Events

#### Client to Server

- **join_room**: Join an existing room
- **upload_audio**: Send audio data for processing

#### Server to Client

- **connected**: Initial connection established
- **reconnected**: Reconnection successful
- **joined_room**: Successfully joined a room
- **audio_received**: Audio file received and being processed
- **transcription**: Audio transcription result
- **error**: Error message
- **user_joined**: New user joined the room
- **user_reconnected**: User reconnected to the room
- **user_disconnected**: User disconnected from the room

## Audio Processing

The backend accepts audio files in WAV format and processes them for transcription. In the current implementation, a mock transcription is returned after a short delay. In a production environment, this would be replaced with an actual transcription service.

## Session Management

Sessions are tracked using cookies and IP addresses. When a user connects, the server attempts to restore their previous session based on:

1. Session ID cookie
2. IP address

Sessions persist for 30 days by default.

## Room Management

Rooms are identified by a 4-character alphanumeric code. Each room has:

- A creator (who can send audio for transcription)
- Listeners (who can only receive transcriptions)

Rooms persist in memory until the server restarts.