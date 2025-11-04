// Create WAV header
function createWavHeader(audioDataSize, sampleRate, channels, bitsPerSample) {
  const byteRate = sampleRate * channels * bitsPerSample / 8;
  const blockAlign = channels * bitsPerSample / 8;
  const header = Buffer.alloc(44);

  // "RIFF" chunk descriptor
  header.write('RIFF', 0);
  header.writeUInt32LE(36 + audioDataSize, 4);
  header.write('WAVE', 8);

  // "fmt " sub-chunk
  header.write('fmt ', 12)
  header.writeUInt32LE(16, 16); // Subchunk1Size
  header.writeUInt16LE(1, 20); // AudioFormat (PCM)
  header.writeUInt16LE(channels, 22);
  header.writeUInt32LE(sampleRate, 24);
  header.writeUInt32LE(byteRate, 28);
  header.writeUInt16LE(blockAlign, 32);
  header.writeUInt16LE(bitsPerSample, 34);

  // "data" sub-chunk
  header.write('data', 36);
  header.writeUInt32LE(audioDataSize, 40);

  return header;
}
async function createWavBuffer(audioBuffer) {
  const totalSize = audioBuffer.reduce((sum, chunk) => sum + chunk.length, 0);
  const header = createWavHeader(totalSize, SAMPLE_RATE, 1, 16);
  const wavBuffer = Buffer.concat([header, ...audioBuffer]);
  
  return wavBuffer;
}
module.exports = {
  createWavBuffer,
  createWavHeader
}