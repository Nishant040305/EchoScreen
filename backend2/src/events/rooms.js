const globals = require("../globals");

module.exports = (io, socket) => {
  socket.on("createRoom", async (sessionId) => {
    try {
      console.log(`User ${sessionId} requested to create a room`);

      let roomId;
      let exists = true;

      // Generate unique room ID
      while (exists) {
        roomId = Math.random().toString(36).substring(2, 6).toUpperCase();
        const existingRoom = await globals.getRoom(roomId);
        if (!existingRoom) exists = false;
      }

      // Store mappings in Redis
      await globals.setRoom(roomId, sessionId);
      await globals.setUser(sessionId, roomId);

      // Join the room
      socket.join(roomId);

      // Notify the creator
      io.to(socket.id).emit("roomId", roomId);
      console.log(`Room ${roomId} created by user ${sessionId}`);
    } catch (error) {
      console.error(`Error creating room for session ${socket.id}:`, error);
      io.to(socket.id).emit("error", "Failed to create room");
    }
  });
};
