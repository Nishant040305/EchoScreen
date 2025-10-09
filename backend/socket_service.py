# socket_service.py
from flask_socketio import emit, join_room as socketio_join_room, leave_room
import os
from transcribe import transcribe_audio_faster_whisper as transcribe_audio
from flask import request
from session_manager import session_manager
from room_service import rooms, lock

UPLOAD_DIR = "uploads"
os.makedirs(UPLOAD_DIR, exist_ok=True)

def handle_connect():
    sid = request.sid
    ip_address = request.remote_addr
    session_id = request.cookies.get('session_id')
    
    print(f"[CONNECT] Client {sid} connected from IP {ip_address}, Session ID: {session_id}")
    
    # First check if we have a session ID from cookie
    if session_id:
        # Try to get session data using the cookie session ID
        session_data = session_manager.get_session_data(session_id)
        if session_data:
            # We found existing session data using the cookie
            room_code = session_data.get('room')
            is_creator = session_data.get('is_creator', False)
            
            # Update the session with the new socket ID
            new_sid, room_code = session_manager.reconnect_session(session_id, sid, ip_address)
            
            if room_code:
                # Rejoin the room
                socketio_join_room(room_code)
                
                print(f"[RECONNECT] Client {sid} reconnected to room {room_code} as {'creator' if is_creator else 'listener'} using cookie session")
                
                # Notify client of successful reconnection
                emit("reconnected", {
                    "sid": sid,
                    "room_code": room_code,
                    "role": "creator" if is_creator else "listener"
                })
                
                # Notify room of reconnection
                emit("user_reconnected", {"sid": sid}, room=room_code, include_self=False)
                return
    
    # If no cookie session or cookie session not found, try IP-based reconnection
    old_sid = session_manager.get_session_by_ip(ip_address)
    
    if old_sid and old_sid != sid:
        # This is a reconnection with a new session ID based on IP
        new_sid, room_code = session_manager.reconnect_session(old_sid, sid, ip_address)
        
        if room_code:
            # Rejoin the room
            socketio_join_room(room_code)
            
            # Get session data to determine role
            session_data = session_manager.get_session_data(sid)
            is_creator = session_data.get('is_creator', False) if session_data else False
            
            print(f"[RECONNECT] Client {sid} reconnected to room {room_code} as {'creator' if is_creator else 'listener'} using IP-based session")
            
            # Notify client of successful reconnection
            emit("reconnected", {
                "sid": sid,
                "room_code": room_code,
                "role": "creator" if is_creator else "listener"
            })
            
            # Notify room of reconnection
            emit("user_reconnected", {"sid": sid}, room=room_code, include_self=False)
            return
    
    # No existing session found, register new session
    session_manager.register_session(sid, ip_address)
    emit("connected", {"sid": sid})

def handle_disconnect():
    sid = request.sid
    
    # Get session data before marking as disconnected
    session_data = session_manager.get_session_data(sid)
    
    if session_data and session_data.get('room'):
        room_code = session_data.get('room')
        is_creator = session_data.get('is_creator', False)
        
        # Mark session as disconnected but keep data for potential reconnection
        session_manager.mark_disconnected(sid)
        
        print(f"[DISCONNECT] Client {sid} disconnected from room {room_code} as {'creator' if is_creator else 'listener'}")
        
        # Notify room of disconnection
        emit("user_disconnected", {"sid": sid}, room=room_code, include_self=False)
    else:
        print(f"[DISCONNECT] Client {sid} disconnected (not in a room)")

def handle_join(data):
    sid = request.sid
    ip_address = request.remote_addr
    room_code = data.get("room_code")
    
    if not room_code:
        emit("error", {"msg": "Room code is required"})
        return
    
    with lock:
        if room_code not in rooms:
            emit("error", {"msg": "Room not found"})
            return
    
    # Check if this user is the creator (by IP or session)
    is_creator = False
    with lock:
        if rooms[room_code].get("creator_sid") == sid:
            is_creator = True
        elif rooms[room_code].get("creator_ip") == ip_address:
            is_creator = True
            # Update creator_sid
            rooms[room_code]["creator_sid"] = sid
    
    # Update session data
    role = "creator" if is_creator else "listener"
    session_manager.update_session(
        sid, 
        room=room_code, 
        is_creator=is_creator,
        user_data={"role": role}
    )
    
    # Add to listeners set if not creator
    with lock:
        if not is_creator:
            rooms[room_code]["listeners"].add(sid)
        rooms[room_code]["participants"] += 1
    
    # Join the socket.io room
    socketio_join_room(room_code)
    
    print(f"[JOIN] Client {sid} joined room {room_code} as {role}")
    emit("joined_room", {
        "room_code": room_code, 
        "role": role,
        "msg": f"Successfully joined room {room_code} as {role}"
    })
    
    # Notify others in the room
    emit("user_joined", {"sid": sid, "role": role}, room=room_code, include_self=False)

def handle_audio_upload(data, socketio):
    """
    data = {"room_code": "ABCD", "filename": "test.wav", "audio": <binary>}
    """
    sid = request.sid
    room_code = data.get("room_code")
    audio_data = data.get("audio")
    
    if not audio_data:
        emit("error", {"msg": "No audio data received"})
        return
    
    if not room_code:
        emit("error", {"msg": "Room code is required"})
        return
    
    # Get session data to verify user is in the room and is the creator
    session_data = session_manager.get_session_data(sid)
    
    if not session_data or session_data.get('room') != room_code:
        emit("error", {"msg": "You are not in this room"})
        return
    
    # Only room creator can send audio for transcription
    if not session_data.get('is_creator', False):
        emit("error", {"msg": "Only the room creator can send audio for transcription"})
        return
    
    # Send immediate acknowledgment to prevent client disconnection
    emit("audio_received", {"status": "processing", "room_code": room_code})
    
    # Process audio in a background thread to avoid blocking the main thread
    def process_audio_in_background():
        filename = os.path.join(UPLOAD_DIR, f"{sid}_{data.get('filename', 'temp.wav')}")

        try:
            # Save audio
            with open(filename, "wb") as f:
                f.write(audio_data)

            print(f"[AUDIO] Saved audio file: {filename}")
            
            # Transcribe
            text = transcribe_audio(filename)
            print(f"[TRANSCRIBED][{room_code}] {text}")

            # Broadcast to room
            socketio.emit("transcription", {
                "text": text, 
                "room_code": room_code,
                "sender": sid,
                "is_creator": True
            }, room=room_code)

            # Clean up
            os.remove(filename)
            print(f"[CLEANUP] Removed {filename}")
            
        except Exception as e:
            print(f"[ERROR] handle_audio_upload: {e}")
            socketio.emit("error", {"msg": f"Transcription failed: {str(e)}"}, room=sid)
    
    # Start background processing thread
    import threading
    audio_thread = threading.Thread(target=process_audio_in_background)
    audio_thread.daemon = True  # Thread will exit when main thread exits
    audio_thread.start()
    
    return "Processing audio..."