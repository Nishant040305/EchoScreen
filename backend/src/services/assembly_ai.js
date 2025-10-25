const fetch = require("node-fetch");
/**
 * Transcribe base64-encoded audio using AssemblyAI
 * @param {string} base64Audio - base64 string (like 'data:audio/webm;base64,...')
 * @param {string} apiKey - your AssemblyAI API key
 * @returns {Promise<string>} - transcribed text
 */
async function transcribeAudio(
  base64Audio,
  apiKey = process.env.ASSEMBLYAI_API_KEY
) {
  if (typeof base64Audio !== "string" || !base64Audio.trim()) {
    throw new Error("Invalid audio data");
  }
  if (!apiKey) {
    throw new Error("Missing AssemblyAI API key");
  }

  const cleaned = base64Audio
    .replace(/^data:audio\/[\w+.-]+;base64,/, "")
    .trim();
  const buffer = Buffer.from(cleaned, "base64");
  if (!buffer.length) throw new Error("Empty audio buffer");

  const uploadRes = await fetch("https://api.assemblyai.com/v2/upload", {
    method: "POST",
    headers: { authorization: apiKey, "transfer-encoding": "chunked" },
    body: buffer,
  });

  if (!uploadRes.ok) {
    const text = await uploadRes.text();
    throw new Error(`Upload failed (${uploadRes.status}): ${text}`);
  }

  const { upload_url } = await uploadRes.json();
  if (!upload_url) throw new Error("Upload URL not received");

  const transcriptRes = await fetch(
    "https://api.assemblyai.com/v2/transcript",
    {
      method: "POST",
      headers: {
        authorization: apiKey,
        "content-type": "application/json",
      },
      body: JSON.stringify({ audio_url: upload_url }),
    }
  );

  if (!transcriptRes.ok) {
    const text = await transcriptRes.text();
    throw new Error(
      `Transcription request failed (${transcriptRes.status}): ${text}`
    );
  }

  const { id } = await transcriptRes.json();
  if (!id) throw new Error("Transcription ID not returned");

  const POLL_INTERVAL = 1500;
  const MAX_WAIT = 120000;
  const start = Date.now();

  while (true) {
    const pollRes = await fetch(
      `https://api.assemblyai.com/v2/transcript/${id}`,
      {
        headers: { authorization: apiKey },
      }
    );

    if (!pollRes.ok) {
      throw new Error(
        `Polling failed (${pollRes.status}): ${pollRes.statusText}`
      );
    }

    const transcript = await pollRes.json();

    if (transcript.status === "completed") {
      return transcript.text;
    }

    if (transcript.status === "error") {
      throw new Error(`Transcription failed: ${transcript.error}`);
    }

    if (Date.now() - start > MAX_WAIT) {
      throw new Error("Transcription timed out");
    }

    await new Promise((r) => setTimeout(r, POLL_INTERVAL));
  }
}

module.exports = transcribeAudio;
