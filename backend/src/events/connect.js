const globals = require("../globals");

module.exports = (io, socket) => {
  socket.on("joinRoom", async ({ roomId, userId }) => {
    try {
      // Fetch data from Redis
      const storedRoomId = await globals.getUser(userId);
      const storedUserId = await globals.getRoom(roomId);

      if (!storedRoomId || !storedUserId) {
        console.log(`User or room not found: ${userId}`);
        return;
      }

      console.log(`User found: ${userId}`);
      await globals.setUser(userId, roomId);
      socket.join(roomId);
      io.to(socket.id).emit("joinedRoom", { roomId, userId });
    } catch (error) {
      console.error(`Error joining chats for user ${userId}:`, error);
      io.to(socket.id).emit("error", "Failed to join room");
    }
  });
  socket.on("connectUser", async () => {
    try {
      console.log(`Socket ${socket.id} connected`);
      let userId;
      let exists = true;

      // Generate unique user ID
      while (exists) {
        userId = Math.random().toString(36).substring(2, 10).toUpperCase();
        const existing = await globals.getUser(userId);
        if (!existing) exists = false;
      }

      await globals.setUser(userId, socket.id);
      io.to(socket.id).emit("userId", userId);
      console.log(`User connected: ${userId} (Socket ${socket.id})`);
    } catch (error) {
      console.error(`Error connecting socket ${socket.id}:`, error);
      io.to(socket.id).emit("error", "Failed to connect user");
    }
  });
};
