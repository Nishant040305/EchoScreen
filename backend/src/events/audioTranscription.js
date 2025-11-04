const fs = require('fs');
const globals = require("../globals");
const transcribeAudio = require("../services/transcribe_audio");

const SAMPLE_RATE = 16000;

// Store audio buffers per socket
const audioBuffers = new Map();
const recordingStates = new Map();

module.exports = (io, socket) => {
  audioBuffers.set(socket.id, []);
  recordingStates.set(socket.id, false);
  socket.on("startAudioRecording", async (data) => {
    try {
      const roomId = await globals.getUser(data.userId);
      const userId = await globals.getRoom(data.roomId);

      if (!roomId || !userId) {
        console.log("User or room not found");
        socket.emit("error", "User or room not found");
        return;
      }

      if (data.roomId !== roomId || data.userId !== userId) {
        console.log("User not in room");
        socket.emit("error", "User not in room");
        return;
      }

      // Start recording
      audioBuffers.set(socket.id, []);
      recordingStates.set(socket.id, true);
      console.log(`🔴 Recording started for user ${data.userId} in room ${data.roomId}`);
      
      socket.emit("recordingStarted", { success: true });
    } catch (error) {
      console.error(`Error starting recording for user ${data.userId}:`, error);
      socket.emit("error", "Failed to start recording");
    }
  });

  // Receive audio chunks
  socket.on("audioChunk", async (data) => {
    try {
      if (!recordingStates.get(socket.id)) {
        return;
      }

      // Handle binary data - convert 32-bit PCM to 16-bit PCM
      const pcm32Buffer = Buffer.from(data.audioData);
      const pcm16Buffer = convertPCM32to16(pcm32Buffer);
      
      // Add to buffer
      const buffer = audioBuffers.get(socket.id);
      if (buffer) {
        buffer.push(pcm16Buffer);
      }
    } catch (error) {
      console.error(`Error processing audio chunk:`, error);
    }
  });

  // Stop recording and transcribe
  socket.on("stopAudioRecording", async (data) => {
    try {
      const roomId = await globals.getUser(data.userId);
      const userId = await globals.getRoom(data.roomId);

      if (!roomId || !userId) {
        console.log("User or room not found");
        socket.emit("error", "User or room not found");
        return;
      }

      if (data.roomId !== roomId || data.userId !== userId) {
        console.log("User not in room");
        socket.emit("error", "User not in room");
        return;
      }

      // Stop recording
      recordingStates.set(socket.id, false);
      console.log(`⏹️ Recording stopped for user ${data.userId}`);

      const buffer = audioBuffers.get(socket.id);
      if (!buffer || buffer.length === 0) {
        console.log("No audio data to transcribe");
        socket.emit("error", "No audio data recorded");
        return;
      }

      console.log("User in room, processing audio...");

      // Save audio as WAV file
      const wavBuffer = await createWavBuffer(buffer);
      
      // Transcribe audio using your existing function
      console.log('🚀 Transcribing audio...');
      const transcribedText = await transcribeAudio(wavBuffer);
      
      console.log("Audio Transcription:", transcribedText);

      // Send transcription to room
      io.to(roomId).emit("audioTranscription", {
        userId: data.userId,
        transcription: transcribedText,
        timestamp: new Date().toISOString()
      });

      // Clean up
      audioBuffers.set(socket.id, []);
      
    } catch (error) {
      console.error(
        `Error processing audio transcription for user ${data.userId}:`,
        error
      );
      socket.emit("error", "Audio transcription failed");
    }
  });

  // Original audioTranscription endpoint (for pre-recorded files)
  socket.on("audioTranscription", async (data) => {
    try {
      const roomId = await globals.getUser(data.userId);
      const userId = await globals.getRoom(data.roomId);

      if (!roomId || !userId) {
        console.log("User or room not found");
        return;
      }

      if (data.roomId !== roomId || data.userId !== userId) {
        console.log("User not in room");
        return;
      }

      console.log("User in room, processing audio file...");

      // Convert audioFile to WAV buffer if needed
      let wavBuffer;
      
      if (Buffer.isBuffer(data.audioFile)) {
        wavBuffer = data.audioFile;
      } else if (typeof data.audioFile === 'string') {
        // Assume it's base64
        wavBuffer = Buffer.from(data.audioFile, 'base64');
      } else {
        throw new Error('Invalid audio file format');
      }

      const transcribedText = await transcribeAudio(wavBuffer);

      console.log("Audio Transcription:", transcribedText);

      io.to(roomId).emit("audioTranscription", {
        userId: data.userId,
        transcription: transcribedText,
        timestamp: new Date().toISOString()
      });
    } catch (error) {
      console.error(
        `Error processing audio transcription for user ${data.userId}:`,
        error
      );
      socket.emit("error", "Audio transcription failed");
    }
  });
};
function convertPCM32to16(pcm32Buffer) {
  const numSamples = pcm32Buffer.length / 4;
  const pcm16Buffer = Buffer.alloc(numSamples * 2);

  for (let i = 0; i < numSamples; i++) {
    const sample32 = pcm32Buffer.readInt32LE(i * 4);
    const sample16 = sample32 >> 14;
    pcm16Buffer.writeInt16LE(sample16, i * 2);
  }

  return pcm16Buffer;
}


