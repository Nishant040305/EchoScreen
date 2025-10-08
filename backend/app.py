try:
    import eventlet
    eventlet.monkey_patch()
    EVENTLET_AVAILABLE = True
except ImportError:
    EVENTLET_AVAILABLE = False

from flask import Flask, request, make_response, jsonify
from flask_socketio import SocketIO
from flask_cors import CORS
import os
import uuid

from room_service import rooms, lock, create_room, join_room_api
from socket_service import handle_connect, handle_disconnect, handle_join, handle_audio_upload
from session_manager import session_manager
from faster_whisper import WhisperModel


app = Flask(__name__)
CORS(app, supports_credentials=True)

socketio = SocketIO(
    app,
    cors_allowed_origins="*",
    async_mode='eventlet' if EVENTLET_AVAILABLE else 'threading',
    manage_session=False,
    allow_credentials=True,
    ping_timeout=60,
    ping_interval=25,
    logger=True,
    engineio_logger=True
)

UPLOAD_DIR = "uploads"
os.makedirs(UPLOAD_DIR, exist_ok=True)

# Register routes
app.add_url_rule("/create_room", "create_room", create_room, methods=["POST"])
app.add_url_rule("/join_room", "join_room_api", join_room_api, methods=["GET"])

@app.route('/session', methods=['GET'])
def get_session():
    session_id = request.cookies.get('session_id')
    ip_address = request.remote_addr

    if not session_id:
        existing_sid = session_manager.get_session_by_ip(ip_address)
        if existing_sid:
            session_id = existing_sid
        else:
            session_id = str(uuid.uuid4())

    session_manager.register_session(session_id, ip_address)

    response = make_response(jsonify({
        'session_id': session_id,
        'status': 'ok'
    }))
    response.set_cookie('session_id', session_id, max_age=30*24*60*60,
                        httponly=True, samesite='Lax')
    return response


# Socket handlers
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
    print("[SERVER] Starting backend on http://0.0.0.0:5000")

    if EVENTLET_AVAILABLE:
        socketio.run(app, host='0.0.0.0', port=5000, debug=False)
    else:
        socketio.run(app, host='0.0.0.0', port=5000, debug=True, allow_unsafe_werkzeug=True)
