const express = require("express");
const path = require("path");
const cors = require("cors");
const { Server } = require("socket.io");
const socketHandlers = require("./events");
const http = require("http");
// Load environment variables
require("dotenv").config();
// Create express app and add middlewares
const app = express();
const server = http.createServer(app);

const io = new Server(server, {
  cors: {
    origin: "*", // Allow all origins
    methods: ["GET", "POST"],
    credentials: true,
    transports: ["websocket", "polling"],
  },
});
socketHandlers(io);
app.use(
  cors({
    origin: "*",
    methods: "GET,POST,PUT,DELETE",
    credentials: true,
  })
);
app.use(express.urlencoded({ extended: true }));
app.use(express.json());

// Middleware for error
app.use((err, req, res, next) => {
  console.error(err.stack);
  res.status(500).send("Internal Server error");
});

const PORT = process.env.PORT || 4000;
server.listen(PORT, () => {
  console.log(`Server and Socket.io are running on port ${PORT}`);
});
