import org.java_websocket.server.WebSocketServer;
import org.java_websocket.WebSocket;
import org.java_websocket.handshake.ClientHandshake;

import javax.sound.sampled.*;
import javax.swing.*;
import java.awt.*;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.io.*;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class AudioWebSocketServerWithPlot extends WebSocketServer {

    private SourceDataLine speaker;
    private final SampleBuffer sampleBuffer;
    private final WaveformPanel waveformPanel;
    private FileOutputStream pcmDumpStream;
    
    // WAV recording
    private List<byte[]> wavBuffer;
    private boolean isRecording = false;
    private static final int SAMPLE_RATE = 16000; // Match your ESP32 configuration
    private static final String ASSEMBLY_AI_API_KEY = "1ea1098916e74b7cb335505f9df441fd"; // Replace with your actual key from assemblyai.com

    public AudioWebSocketServerWithPlot(int port) throws LineUnavailableException {
        super(new InetSocketAddress(port));
        sampleBuffer = new SampleBuffer(48000);
        waveformPanel = new WaveformPanel(sampleBuffer);
        wavBuffer = new ArrayList<>();

        try {
            pcmDumpStream = new FileOutputStream("pcm_dump.raw", true);
        } catch (IOException e) {
            e.printStackTrace();
        }

        setupSpeaker();
        setupUI();
    }

    private void setupSpeaker() throws LineUnavailableException {
        AudioFormat format = new AudioFormat(
                SAMPLE_RATE,
                16,
                1,
                true,
                false
        );

        DataLine.Info info = new DataLine.Info(SourceDataLine.class, format);
        speaker = (SourceDataLine) AudioSystem.getLine(info);
        speaker.open(format);
        speaker.start();
        System.out.println("🎧 Speaker ready for PCM audio (16-bit @ " + SAMPLE_RATE + "Hz)");
    }

    private void setupUI() {
        SwingUtilities.invokeLater(() -> {
            JFrame frame = new JFrame("ESP32 Audio Waveform — WebSocket Server");
            frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
            frame.setLayout(new BorderLayout());
            
            // Add control panel
            JPanel controlPanel = new JPanel();
            JButton recordButton = new JButton("Start Recording");
            JButton stopButton = new JButton("Stop & Transcribe");
            stopButton.setEnabled(false);
            
            recordButton.addActionListener(e -> {
                startRecording();
                recordButton.setEnabled(false);
                stopButton.setEnabled(true);
            });
            
            stopButton.addActionListener(e -> {
                stopRecordingAndTranscribe();
                recordButton.setEnabled(true);
                stopButton.setEnabled(false);
            });
            
            controlPanel.add(recordButton);
            controlPanel.add(stopButton);
            
            frame.add(controlPanel, BorderLayout.NORTH);
            frame.add(waveformPanel, BorderLayout.CENTER);
            frame.setSize(800, 350);
            frame.setLocationRelativeTo(null);
            frame.setVisible(true);

            Timer timer = new Timer(25, e -> waveformPanel.repaint());
            timer.start();
        });
    }

    private void startRecording() {
        wavBuffer.clear();
        isRecording = true;
        System.out.println("🔴 Recording started...");
    }

    private void stopRecordingAndTranscribe() {
        isRecording = false;
        System.out.println("⏹️ Recording stopped. Saving WAV file...");
        
        // Run transcription in background thread to avoid blocking UI
        new Thread(() -> {
            try {
                String wavFilePath = "recording_" + System.currentTimeMillis() + ".wav";
                saveAsWav(wavFilePath);
                System.out.println("✅ WAV file saved: " + wavFilePath);
                
                System.out.println("🚀 Sending to AssemblyAI for transcription...");
                transcribeWithAssemblyAI(wavFilePath);
                
            } catch (Exception e) {
                System.err.println("❌ Error during transcription: " + e.getMessage());
                e.printStackTrace();
            }
        }).start();
    }

    private void saveAsWav(String filename) throws IOException {
        // Calculate total audio data size
        int totalSize = 0;
        for (byte[] chunk : wavBuffer) {
            totalSize += chunk.length;
        }

        FileOutputStream fos = new FileOutputStream(filename);
        
        // Write WAV header
        writeWavHeader(fos, totalSize, SAMPLE_RATE, 1, 16);
        
        // Write audio data
        for (byte[] chunk : wavBuffer) {
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
        fos.write(intToBytes(16)); // fmt chunk size
        fos.write(shortToBytes((short) 1)); // PCM format
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

    // Simple JSON parser for our specific needs
    private String extractJsonValue(String json, String key) {
        Pattern pattern = Pattern.compile("\"" + key + "\"\\s*:\\s*\"([^\"]+)\"");
        Matcher matcher = pattern.matcher(json);
        if (matcher.find()) {
            return matcher.group(1);
        }
        return null;
    }

    private void transcribeWithAssemblyAI(String wavFilePath) throws Exception {
        // Step 1: Upload file to AssemblyAI
        String uploadUrl = uploadFile(wavFilePath);
        System.out.println("📤 File uploaded. URL: " + uploadUrl);
        
        // Step 2: Request transcription
        String transcriptId = requestTranscription(uploadUrl);
        System.out.println("🔄 Transcription requested. ID: " + transcriptId);
        
        // Step 3: Poll for transcription result
        String transcription = pollTranscription(transcriptId);
        System.out.println("\n📝 TRANSCRIPTION RESULT:");
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
            throw new Exception("Failed to get upload URL from response");
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
            throw new Exception("Failed to get transcript ID from response");
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
                    throw new Exception("Failed to extract transcription text");
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

    @Override
    public void onOpen(WebSocket conn, ClientHandshake handshake) {
        System.out.println("✅ Client connected: " + conn.getRemoteSocketAddress());
    }

    @Override
    public void onClose(WebSocket conn, int code, String reason, boolean remote) {
        System.out.println("❌ Client disconnected: " + conn.getRemoteSocketAddress());
    }

    @Override
    public void onMessage(WebSocket conn, String message) {
        // Ignore text messages
    }

    @Override
    public void onMessage(WebSocket conn, ByteBuffer message) {
        byte[] pcm32Bytes = message.array();

        int numSamples = pcm32Bytes.length / 4;
        short[] pcm16 = new short[numSamples];

        for (int i = 0; i < numSamples; i++) {
            int sample32 = (pcm32Bytes[i * 4] & 0xFF) |
                           ((pcm32Bytes[i * 4 + 1] & 0xFF) << 8) |
                           ((pcm32Bytes[i * 4 + 2] & 0xFF) << 16) |
                           ((pcm32Bytes[i * 4 + 3] & 0xFF) << 24);
            pcm16[i] = (short) (sample32 >> 14);
        }

        byte[] pcmBytes = new byte[pcm16.length * 2];
        for (int i = 0; i < pcm16.length; i++) {
            pcmBytes[i * 2] = (byte) (pcm16[i] & 0xFF);
            pcmBytes[i * 2 + 1] = (byte) ((pcm16[i] >> 8) & 0xFF);
        }

        if (speaker != null) {
            speaker.write(pcmBytes, 0, pcmBytes.length);
        }

        sampleBuffer.pushSamples(pcm16, pcm16.length);

        // Add to WAV buffer if recording
        if (isRecording) {
            wavBuffer.add(pcmBytes.clone());
        }

        try {
            if (pcmDumpStream != null) {
                pcmDumpStream.write(pcmBytes);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onError(WebSocket conn, Exception ex) {
        ex.printStackTrace();
    }

    @Override
    public void onStart() {
        System.out.println("🚀 Server started on port: " + getPort());
        System.out.println("💡 Click 'Start Recording' to record audio for transcription");
    }

    public static void main(String[] args) throws Exception {
        int port = 8080;
        AudioWebSocketServerWithPlot server = new AudioWebSocketServerWithPlot(port);
        server.start();
    }

    // ===== SampleBuffer =====
    static class SampleBuffer {
        private final short[] buffer;
        private int writePos = 0;
        private int size = 0;

        public SampleBuffer(int capacity) {
            buffer = new short[capacity];
        }

        public synchronized void pushSamples(short[] samples, int length) {
            for (int i = 0; i < length; i++) {
                buffer[writePos] = samples[i];
                writePos = (writePos + 1) % buffer.length;
                if (size < buffer.length) size++;
            }
        }

        public synchronized short[] getLastSamples(int n) {
            int take = Math.min(n, size);
            short[] out = new short[take];
            int start = (writePos - take + buffer.length) % buffer.length;
            if (start + take <= buffer.length) {
                System.arraycopy(buffer, start, out, 0, take);
            } else {
                int firstLen = buffer.length - start;
                System.arraycopy(buffer, start, out, 0, firstLen);
                System.arraycopy(buffer, 0, out, firstLen, take - firstLen);
            }
            return out;
        }
    }

    // ===== WaveformPanel =====
    static class WaveformPanel extends JPanel {
        private final SampleBuffer sampleBuffer;
        private double yScale = 1.0;
        private int xZoom = 1;
        private boolean showGrid = true;

        public WaveformPanel(SampleBuffer sampleBuffer) {
            this.sampleBuffer = sampleBuffer;
            setBackground(Color.BLACK);

            addMouseWheelListener(e -> {
                if (e.isControlDown()) {
                    yScale += e.getPreciseWheelRotation() * -0.1;
                    yScale = Math.max(0.1, Math.min(yScale, 5.0));
                } else {
                    xZoom += e.getWheelRotation();
                    xZoom = Math.max(1, xZoom);
                }
                repaint();
            });
        }

        @Override
        protected void paintComponent(Graphics g) {
            super.paintComponent(g);
            int w = getWidth();
            int h = getHeight();
            if (w <= 0 || h <= 0) return;

            short[] samples = sampleBuffer.getLastSamples(w * xZoom);
            if (samples == null || samples.length == 0) {
                g.setColor(Color.WHITE);
                g.drawString("Waiting for audio...", 10, 20);
                return;
            }

            Graphics2D g2 = (Graphics2D) g;
            g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);

            if (showGrid) {
                g2.setColor(Color.DARK_GRAY);
                for (int i = 0; i <= 4; i++) {
                    int y = h / 4 * i;
                    g2.drawLine(0, y, w, y);
                }
                for (int i = 0; i <= w; i += 50) {
                    g2.drawLine(i, 0, i, h);
                }
            }

            g2.setColor(Color.GREEN);
            int prevX = 0, prevY = h / 2;
            double vScale = (h / 2.0) / 32768.0 * yScale;

            for (int x = 0; x < w; x++) {
                int sampleIndex = x * xZoom;
                if (sampleIndex >= samples.length) break;
                double val = samples[sampleIndex];
                int y = (int) (h / 2 - val * vScale);
                if (x > 0) g2.drawLine(prevX, prevY, x, y);
                prevX = x;
                prevY = y;
            }

            g2.setColor(Color.WHITE);
            g2.drawString("Samples: " + samples.length, 10, 20);
            g2.drawString(String.format("Y-scale: %.2f, X-zoom: %d", yScale, xZoom), 10, 35);
        }
    }
}