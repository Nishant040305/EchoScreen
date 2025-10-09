from faster_whisper import WhisperModel
import requests
import time
import os
from dotenv import load_dotenv

load_dotenv()

assemblyai_api_key = os.getenv("ASSEMBLYAI_API_KEY")


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

def transcribe_audio_assembly(file_path: str = None, audio_url: str = None, language: str = None) -> str:
    """
    Transcribes an audio file using the AssemblyAI API.

    Args:
        file_path (str): Local path to the audio file (optional if audio_url is provided).
        audio_url (str): Direct URL to an audio file (optional if file_path is provided).
        language (str): Optional language code for the transcription.

    Returns:
        str: Transcribed text from the audio.
    """

    base_url = "https://api.assemblyai.com/v2"
    headers = {
        "authorization": assemblyai_api_key  
    }

    # Upload local file if file_path is provided
    if file_path and not audio_url:
        with open(file_path, "rb") as f:
            upload_response = requests.post(f"{base_url}/upload", headers=headers, data=f)
        upload_response.raise_for_status()
        audio_url = upload_response.json()["upload_url"]

    if not audio_url:
        raise ValueError("Either file_path or audio_url must be provided.")

    # Request transcription
    data = {
        "audio_url": audio_url,
        "speech_model": "universal"
    }
    if language:
        data["language_code"] = language

    transcript_response = requests.post(f"{base_url}/transcript", json=data, headers=headers)
    transcript_response.raise_for_status()
    transcript_id = transcript_response.json()["id"]

    polling_endpoint = f"{base_url}/transcript/{transcript_id}"

    # Poll until completion
    while True:
        polling_response = requests.get(polling_endpoint, headers=headers)
        polling_response.raise_for_status()
        transcription_result = polling_response.json()

        if transcription_result["status"] == "completed":
            return transcription_result["text"]

        elif transcription_result["status"] == "error":
            raise RuntimeError(f"Transcription failed: {transcription_result['error']}")

        time.sleep(3)
