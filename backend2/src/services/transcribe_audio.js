const assemblyai_text = require("./assembly_ai");
async function transcribeAudio(buffer) {
  text = await assemblyai_text(buffer, process.env.ASSEMBLYAI_API_KEY);
  return text;
}
module.exports = transcribeAudio;
