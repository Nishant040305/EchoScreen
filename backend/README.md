# ESP-32 Morse-Based Chat Application Backend

## Overview

This is the backend for a real-time audio transcription service with room functionality. The system allows users to create rooms where one user is designated as the creator (speaker) and others join as listeners. The creator can send audio which is transcribed and broadcast to all listeners in the room.

## Features

- **Room Management**: Create and join rooms using 4-digit codes
- **User Roles**: Distinct creator (speaker) and listener roles
- **Audio Transcription**: Real-time speech-to-text conversion
- **Persistent Sessions**: Handles disconnections gracefully with session tracking
- **Reconnection Logic**: Users can reconnect to their rooms after connection drops

## Architecture

### Components

- **Flask Server**: Handles HTTP requests for room creation and joining
- **Socket.IO**: Manages real-time communication between clients
- **Session Manager**: Tracks user sessions and handles reconnections
- **Room Service**: Manages room creation and access
- **Transcription Service**: Converts audio to text using Faster Whisper

## Setup and Installation

### Prerequisites

- Python 3.7+
- pip

### Installation

1. Clone the repository
2. Install dependencies:

```bash
pip install flask flask-socketio flask-cors faster-whisper
```

### Running the Server

```bash
python app.py
```

The server will start on http://localhost:5000

## API Endpoints

### Create Room

- **URL**: `/create_room`
- **Method**: `POST`
- **Response**: `{"room_code": "XXXX", "created_at": "timestamp"}`

### Join Room

- **URL**: `/join_room?code=XXXX`
- **Method**: `GET`
- **Response**: `{"status": "ok", "room_code": "XXXX", "role": "creator|listener"}`

### Get Session

- **URL**: `/session`
- **Method**: `GET`
- **Response**: `{"session_id": "uuid", "status": "ok"}`

## Socket.IO Events

### Client to Server

- `connect`: Connect to the server
- `disconnect`: Disconnect from the server
- `join_room`: Join a room with a room code
- `upload_audio`: Upload audio for transcription (creator only)

### Server to Client

- `connected`: Confirmation of connection
- `reconnected`: Confirmation of reconnection with room details
- `joined_room`: Confirmation of joining a room
- `transcription`: Transcribed text from audio
- `user_joined`: Notification when a user joins the room
- `user_disconnected`: Notification when a user disconnects
- `user_reconnected`: Notification when a user reconnects
- `error`: Error messages

## Handling Disconnections

The system uses a combination of techniques to handle disconnections:

1. **Session Tracking**: Maps IP addresses and session IDs to user data
2. **Reconnection Logic**: Automatically rejoins rooms on reconnection
3. **Role Persistence**: Maintains user roles (creator/listener) across reconnections

## Testing

A test HTML interface is provided in the `test` directory. Open `test.html` in a browser to test the functionality.

## License

MIT