# room_service.py
from flask import request, jsonify
from datetime import datetime
import random
import string
import threading
from session_manager import session_manager

rooms = {}
lock = threading.Lock()

def generate_room_code(length=4):
    return ''.join(random.choices(string.ascii_uppercase + string.digits, k=length))

def create_room():
    # Get client IP and session ID
    ip_address = request.remote_addr
    sid = request.cookies.get('session_id')
    
    with lock:
        code = generate_room_code()
        while code in rooms:
            code = generate_room_code()
        
        rooms[code] = {
            "created_at": datetime.utcnow().isoformat(),
            "participants": 0,
            "creator_sid": sid,
            "creator_ip": ip_address,
            "listeners": set()
        }
        
        # Update session data if we have a session ID
        if sid:
            session_manager.update_session(
                sid, 
                room=code, 
                is_creator=True,
                user_data={"role": "creator"}
            )
    
    print(f"[ROOM] Created room: {code} by {sid} ({ip_address})")
    return jsonify({"room_code": code, "created_at": rooms[code]["created_at"]})

def join_room_api():
    code = request.args.get("code", "").upper()
    if not code:
        return jsonify({"status": "error", "message": "Room code required"}), 400
    
    # Get client IP and session ID
    ip_address = request.remote_addr
    sid = request.cookies.get('session_id')
    
    with lock:
        if code in rooms:
            # Check if this user is the creator (by IP or session)
            is_creator = False
            if sid and rooms[code].get("creator_sid") == sid:
                is_creator = True
            elif ip_address and rooms[code].get("creator_ip") == ip_address:
                is_creator = True
                # Update creator_sid if we have a new one
                if sid and not rooms[code].get("creator_sid"):
                    rooms[code]["creator_sid"] = sid
            
            # Update session data if we have a session ID
            if sid:
                role = "creator" if is_creator else "listener"
                session_manager.update_session(
                    sid, 
                    room=code, 
                    is_creator=is_creator,
                    user_data={"role": role}
                )
                
                # Add to listeners set if not creator
                if not is_creator and sid not in rooms[code]["listeners"]:
                    rooms[code]["listeners"].add(sid)
            
            return jsonify({
                "status": "ok", 
                "room_code": code, 
                "role": "creator" if is_creator else "listener"
            })
        
        return jsonify({"status": "error", "message": "Room not found"}), 404