from faster_whisper import WhisperModel

def transcribe_audio_faster_whisper(
    file_path: str, 
    model_size: str = "base",
    language: str = None
) -> str:
    """
    Best free option - Fast, accurate, completely offline.
    Requires: pip install faster-whisper
    Model sizes: tiny, base, small, medium, large-v2, large-v3
    First run downloads model automatically (stored locally).
    """
    try:
        # Use GPU if available, otherwise CPU
        model = WhisperModel(model_size, device="cpu", compute_type="int8")
        
        segments, info = model.transcribe(
            file_path, 
            language=language,
            beam_size=5
        )
        
        print(f"Detected language: {info.language} (probability: {info.language_probability:.2f})")
        
        transcript = " ".join([segment.text for segment in segments])
        return transcript.strip()
    except Exception as e:
        print(f"[ERROR][transcribe_audio_faster_whisper] {e}")
        return ""

def transcribe_audio(file_path: str, language: str = None) -> str:
    return transcribe_audio_faster_whisper(file_path, language=language)