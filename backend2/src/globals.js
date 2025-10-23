// src/services/globals.js
const redis = require("./config/redisClient");

// Keys
const ROOM_KEY = "chat:rooms";
const USER_KEY = "chat:users";

async function setRoom(roomId, userId) {
  await redis.hSet(ROOM_KEY, roomId, userId);
}

async function getRoom(roomId) {
  return await redis.hGet(ROOM_KEY, roomId);
}

async function deleteRoom(roomId) {
  await redis.hDel(ROOM_KEY, roomId);
}

async function setUser(userId, roomId) {
  await redis.hSet(USER_KEY, userId, roomId);
}

async function getUser(userId) {
  return await redis.hGet(USER_KEY, userId);
}

async function deleteUser(userId) {
  await redis.hDel(USER_KEY, userId);
}

module.exports = {
  setRoom,
  getRoom,
  deleteRoom,
  setUser,
  getUser,
  deleteUser,
};
