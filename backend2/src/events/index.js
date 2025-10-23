const connect = require("./connect");
const audioTranscription = require("./audioTranscription");
const roomSocket = require("./rooms");

module.exports = (io) => {
  io.on("connection", (socket) => {
    connect(io, socket);
    audioTranscription(io, socket);
    roomSocket(io, socket);
    socket.on("disconnect", () => {
      console.log("A user disconnected:", socket.id);
    });
  });
};
