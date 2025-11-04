import org.java_websocket.server.WebSocketServer;
import org.java_websocket.WebSocket;
import org.java_websocket.handshake.ClientHandshake;

import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.io.*;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class AudioWebSocketServer extends WebSocketServer {

    private List<byte[]> audioBuffer;
    private boolean isRecording = false;
    private static final int SAMPLE_RATE = 16000;
    private static final String ASSEMBLY_AI_API_KEY = "1ea1098916e74b7cb335505f9df441fd"; // Replace with your key

    public AudioWebSocketServer(int port) {
        super(new InetSocketAddress(port));
        audioBuffer = new ArrayList<>();
    }

    @Override
    public void onOpen(WebSocket conn, ClientHandshake handshake) {
        System.out.println("✅ Client connected: " + conn.getRemoteSocketAddress());
        startRecording();
    }

    @Override
    public void onClose(WebSocket conn, int code, String reason, boolean remote) {
        System.out.println("❌ Client disconnected: " + conn.getRemoteSocketAddress());
        stopRecordingAndTranscribe();
    }

    @Override
    public void onMessage(WebSocket conn, String message) {
        // Ignore text messages
    }

    @Override
    public void onMessage(WebSocket conn, ByteBuffer message) {
        byte[] pcm32Bytes = message.array();

        // Convert 32-bit PCM to 16-bit PCM
        int numSamples = pcm32Bytes.length / 4;
        short[] pcm16 = new short[numSamples];

        for (int i = 0; i < numSamples; i++) {
            int sample32 = (pcm32Bytes[i * 4] & 0xFF) |
                           ((pcm32Bytes[i * 4 + 1] & 0xFF) << 8) |
                           ((pcm32Bytes[i * 4 + 2] & 0xFF) << 16) |
                           ((pcm32Bytes[i * 4 + 3] & 0xFF) << 24);
            pcm16[i] = (short) (sample32 >> 14);
        }

        // Convert to byte array
        byte[] pcmBytes = new byte[pcm16.length * 2];
        for (int i = 0; i < pcm16.length; i++) {
            pcmBytes[i * 2] = (byte) (pcm16[i] & 0xFF);
            pcmBytes[i * 2 + 1] = (byte) ((pcm16[i] >> 8) & 0xFF);
        }

        // Add to buffer if recording
        if (isRecording) {
            audioBuffer.add(pcmBytes.clone());
        }
    }

    @Override
    public void onError(WebSocket conn, Exception ex) {
        ex.printStackTrace();
    }

    @Override
    public void onStart() {
        System.out.println("🚀 Server started on port: " + getPort());
        System.out.println("💡 Waiting for client connection...");
    }

    private void startRecording() {
        audioBuffer.clear();
        isRecording = true;
        System.out.println("🔴 Recording started...");
    }

    private void stopRecordingAndTranscribe() {
        isRecording = false;
        System.out.println("⏹️ Recording stopped. Processing audio...");
        
        new Thread(() -> {
            try {
                String wavFilePath = "recording_" + System.currentTimeMillis() + ".wav";
                saveAsWav(wavFilePath);
                System.out.println("✅ WAV file saved: " + wavFilePath);
                
                System.out.println("🚀 Sending to AssemblyAI for transcription...");
                transcribeWithAssemblyAI(wavFilePath);
                
            } catch (Exception e) {
                System.err.println("❌ Error: " + e.getMessage());
                e.printStackTrace();
            }
        }).start();
    }

    private void saveAsWav(String filename) throws IOException {
        int totalSize = 0;
        for (byte[] chunk : audioBuffer) {
            totalSize += chunk.length;
        }

        FileOutputStream fos = new FileOutputStream(filename);
        
        // Write WAV header
        writeWavHeader(fos, totalSize, SAMPLE_RATE, 1, 16);
        
        // Write audio data
        for (byte[] chunk : audioBuffer) {
            fos.write(chunk);
        }
        
        fos.close();
    }

    private void writeWavHeader(FileOutputStream fos, int audioDataSize, int sampleRate, 
                                 int channels, int bitsPerSample) throws IOException {
        int byteRate = sampleRate * channels * bitsPerSample / 8;
        int blockAlign = channels * bitsPerSample / 8;
        
        fos.write("RIFF".getBytes());
        fos.write(intToBytes(36 + audioDataSize));
        fos.write("WAVE".getBytes());
        fos.write("fmt ".getBytes());
        fos.write(intToBytes(16));
        fos.write(shortToBytes((short) 1));
        fos.write(shortToBytes((short) channels));
        fos.write(intToBytes(sampleRate));
        fos.write(intToBytes(byteRate));
        fos.write(shortToBytes((short) blockAlign));
        fos.write(shortToBytes((short) bitsPerSample));
        fos.write("data".getBytes());
        fos.write(intToBytes(audioDataSize));
    }

    private byte[] intToBytes(int value) {
        return new byte[] {
            (byte) (value & 0xFF),
            (byte) ((value >> 8) & 0xFF),
            (byte) ((value >> 16) & 0xFF),
            (byte) ((value >> 24) & 0xFF)
        };
    }

    private byte[] shortToBytes(short value) {
        return new byte[] {
            (byte) (value & 0xFF),
            (byte) ((value >> 8) & 0xFF)
        };
    }

    private String extractJsonValue(String json, String key) {
        Pattern pattern = Pattern.compile("\"" + key + "\"\\s*:\\s*\"([^\"]+)\"");
        Matcher matcher = pattern.matcher(json);
        if (matcher.find()) {
            return matcher.group(1);
        }
        return null;
    }

    private void transcribeWithAssemblyAI(String wavFilePath) throws Exception {
        String uploadUrl = uploadFile(wavFilePath);
        System.out.println("📤 File uploaded: " + uploadUrl);
        
        String transcriptId = requestTranscription(uploadUrl);
        System.out.println("🔄 Transcription ID: " + transcriptId);
        
        String transcription = pollTranscription(transcriptId);
        System.out.println("\n📝 TRANSCRIPTION:");
        System.out.println("═══════════════════════════════════════");
        System.out.println(transcription);
        System.out.println("═══════════════════════════════════════\n");
    }

    private String uploadFile(String filePath) throws Exception {
        File file = new File(filePath);
        URL url = new URL("https://api.assemblyai.com/v2/upload");
        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        conn.setRequestMethod("POST");
        conn.setRequestProperty("authorization", ASSEMBLY_AI_API_KEY);
        conn.setDoOutput(true);
        
        try (FileInputStream fis = new FileInputStream(file);
             OutputStream os = conn.getOutputStream()) {
            byte[] buffer = new byte[8192];
            int bytesRead;
            while ((bytesRead = fis.read(buffer)) != -1) {
                os.write(buffer, 0, bytesRead);
            }
        }
        
        BufferedReader br = new BufferedReader(new InputStreamReader(conn.getInputStream()));
        StringBuilder response = new StringBuilder();
        String line;
        while ((line = br.readLine()) != null) {
            response.append(line);
        }
        br.close();
        
        String uploadUrl = extractJsonValue(response.toString(), "upload_url");
        if (uploadUrl == null) {
            throw new Exception("Failed to get upload URL");
        }
        return uploadUrl;
    }

    private String requestTranscription(String audioUrl) throws Exception {
        URL url = new URL("https://api.assemblyai.com/v2/transcript");
        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        conn.setRequestMethod("POST");
        conn.setRequestProperty("authorization", ASSEMBLY_AI_API_KEY);
        conn.setRequestProperty("Content-Type", "application/json");
        conn.setDoOutput(true);
        
        String requestBody = "{\"audio_url\":\"" + audioUrl + "\"}";
        
        try (OutputStream os = conn.getOutputStream()) {
            os.write(requestBody.getBytes());
        }
        
        BufferedReader br = new BufferedReader(new InputStreamReader(conn.getInputStream()));
        StringBuilder response = new StringBuilder();
        String line;
        while ((line = br.readLine()) != null) {
            response.append(line);
        }
        br.close();
        
        String transcriptId = extractJsonValue(response.toString(), "id");
        if (transcriptId == null) {
            throw new Exception("Failed to get transcript ID");
        }
        return transcriptId;
    }

    private String pollTranscription(String transcriptId) throws Exception {
        URL url = new URL("https://api.assemblyai.com/v2/transcript/" + transcriptId);
        
        while (true) {
            HttpURLConnection conn = (HttpURLConnection) url.openConnection();
            conn.setRequestMethod("GET");
            conn.setRequestProperty("authorization", ASSEMBLY_AI_API_KEY);
            
            BufferedReader br = new BufferedReader(new InputStreamReader(conn.getInputStream()));
            StringBuilder response = new StringBuilder();
            String line;
            while ((line = br.readLine()) != null) {
                response.append(line);
            }
            br.close();
            
            String responseStr = response.toString();
            String status = extractJsonValue(responseStr, "status");
            
            if ("completed".equals(status)) {
                String text = extractJsonValue(responseStr, "text");
                if (text == null) {
                    throw new Exception("Failed to extract transcription");
                }
                return text;
            } else if ("error".equals(status)) {
                String error = extractJsonValue(responseStr, "error");
                throw new Exception("Transcription failed: " + error);
            }
            
            System.out.println("⏳ Status: " + status + " - waiting...");
            Thread.sleep(3000);
        }
    }

    public static void main(String[] args) throws Exception {
        int port = 8080;
        AudioWebSocketServer server = new AudioWebSocketServer(port);
        server.start();
    }
}