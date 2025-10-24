const globals = require("../globals");
const transcribeAudio = require("../services/transcribe_audio");

module.exports = (io, socket) => {
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

      console.log("User in room, processing audio...");

      const transcribedText = await transcribeAudio(data.audioFile);

      console.log("Audio Transcription:", transcribedText);

      io.to(roomId).emit("audioTranscription", transcribedText);
    } catch (error) {
      console.error(
        `Error processing audio transcription for user ${data.userId}:`,
        error
      );
      io.to(socket.id).emit("error", "Audio transcription failed");
    }
  });
};
