# app.py
from flask import Flask, request, make_response, jsonify
from flask_socketio import SocketIO
from flask_cors import CORS
from room_service import rooms, lock
from socket_service import handle_connect, handle_disconnect, handle_join, handle_audio_upload
from session_manager import session_manager
import os
import uuid
from faster_whisper import WhisperModel

app = Flask(__name__)
CORS(app, supports_credentials=True)
# Configure SocketIO with ping_timeout and ping_interval to maintain connections
socketio = SocketIO(
    app, 
    cors_allowed_origins="*", 
    async_mode='threading', 
    manage_session=False, 
    allow_credentials=True,
    ping_timeout=60,  # Increase ping timeout to 60s
    ping_interval=25,  # Send ping every 25s to keep connection alive
    logger=True,      # Enable logging for debugging
    engineio_logger=True  # Enable Engine.IO logging
)

UPLOAD_DIR = "uploads"
os.makedirs(UPLOAD_DIR, exist_ok=True)

# Import room routes
from room_service import create_room, join_room_api
app.add_url_rule("/create_room", "create_room", create_room, methods=["POST"])
app.add_url_rule("/join_room", "join_room_api", join_room_api, methods=["GET"])

@app.route('/session', methods=['GET'])
def get_session():
    # Check if client has a session cookie
    session_id = request.cookies.get('session_id')
    ip_address = request.remote_addr
    
    # If no session cookie, check if IP has a session
    if not session_id:
        existing_sid = session_manager.get_session_by_ip(ip_address)
        if existing_sid:
            session_id = existing_sid
        else:
            # Create new session ID
            session_id = str(uuid.uuid4())
    
    # Register or update session
    session_manager.register_session(session_id, ip_address)
    
    # Create response with session info
    response = make_response(jsonify({
        'session_id': session_id,
        'status': 'ok'
    }))
    
    # Set session cookie (30 days expiry)
    response.set_cookie('session_id', session_id, max_age=30*24*60*60, httponly=True, samesite='Lax')
    
    return response

# Register socket handlers
@socketio.on("connect")
def on_connect():
    return handle_connect()

@socketio.on("disconnect")
def on_disconnect():
    return handle_disconnect()

@socketio.on("join_room")
def on_join(data):
    return handle_join(data)

@socketio.on("upload_audio")
def on_upload_audio(data):
    return handle_audio_upload(data, socketio)

if __name__ == "__main__":
    print("[SERVER] Starting backend on http://localhost:5000")
    socketio.run(app, host='0.0.0.0', port=5000, debug=True)