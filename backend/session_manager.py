# session_manager.py
import threading
from datetime import datetime, timedelta
import time

class SessionManager:
    def __init__(self, cleanup_interval=300):  # 5 minutes cleanup interval
        self.sessions = {}  # Maps session_id/sid to user data
        self.ip_to_sid = {}  # Maps IP addresses to session IDs
        self.lock = threading.Lock()
        self.cleanup_interval = cleanup_interval
        self.start_cleanup_thread()
    
    def start_cleanup_thread(self):
        """Start a background thread to clean up expired sessions"""
        def cleanup_worker():
            while True:
                time.sleep(self.cleanup_interval)
                self.cleanup_expired_sessions()
        
        cleanup_thread = threading.Thread(target=cleanup_worker, daemon=True)
        cleanup_thread.start()
    
    def register_session(self, sid, ip_address, user_data=None):
        """Register a new session or update existing one"""
        with self.lock:
            # Store session data
            self.sessions[sid] = {
                'ip_address': ip_address,
                'user_data': user_data or {},
                'connected': True,
                'last_seen': datetime.utcnow(),
                'room': None,
                'is_creator': False
            }
            
            # Map IP to this session ID
            self.ip_to_sid[ip_address] = sid
            
            return sid
    
    def get_session_by_ip(self, ip_address):
        """Get session ID by IP address"""
        with self.lock:
            return self.ip_to_sid.get(ip_address)
    
    def get_session_data(self, sid):
        """Get session data by session ID"""
        with self.lock:
            return self.sessions.get(sid)
    
    def update_session(self, sid, **kwargs):
        """Update session data"""
        with self.lock:
            if sid in self.sessions:
                for key, value in kwargs.items():
                    if key == 'user_data' and isinstance(value, dict):
                        # Merge user_data dictionaries
                        self.sessions[sid]['user_data'].update(value)
                    else:
                        self.sessions[sid][key] = value
                
                # Always update last_seen timestamp
                self.sessions[sid]['last_seen'] = datetime.utcnow()
                return True
            return False
    
    def mark_disconnected(self, sid):
        """Mark a session as disconnected but keep its data"""
        with self.lock:
            if sid in self.sessions:
                self.sessions[sid]['connected'] = False
                self.sessions[sid]['last_seen'] = datetime.utcnow()
                return True
            return False
    
    def reconnect_session(self, old_sid, new_sid, ip_address):
        """Handle reconnection with a new session ID"""
        with self.lock:
            if old_sid in self.sessions:
                # Copy data from old session
                session_data = self.sessions[old_sid]
                
                # Create new session with old data
                self.sessions[new_sid] = {
                    'ip_address': ip_address,
                    'user_data': session_data['user_data'],
                    'connected': True,
                    'last_seen': datetime.utcnow(),
                    'room': session_data['room'],
                    'is_creator': session_data['is_creator']
                }
                
                # Update IP mapping
                self.ip_to_sid[ip_address] = new_sid
                
                # Clean up old session
                del self.sessions[old_sid]
                
                return new_sid, session_data['room']
            return None, None
    
    def cleanup_expired_sessions(self, expiry_minutes=30):
        """Remove sessions that have been disconnected for too long"""
        expiry_time = datetime.utcnow() - timedelta(minutes=expiry_minutes)
        
        with self.lock:
            expired_sids = []
            
            for sid, data in self.sessions.items():
                if not data['connected'] and data['last_seen'] < expiry_time:
                    expired_sids.append(sid)
                    
                    # Also remove from IP mapping if it still points to this session
                    ip = data['ip_address']
                    if ip in self.ip_to_sid and self.ip_to_sid[ip] == sid:
                        del self.ip_to_sid[ip]
            
            # Remove expired sessions
            for sid in expired_sids:
                del self.sessions[sid]
            
            if expired_sids:
                print(f"[SESSION] Cleaned up {len(expired_sids)} expired sessions")

# Create a global instance
session_manager = SessionManager()