// silence method - 1
import org.java_websocket.server.WebSocketServer;
import org.java_websocket.WebSocket;
import org.java_websocket.handshake.ClientHandshake;

import io.socket.client.IO;
import io.socket.client.Socket;
import org.json.JSONObject;

import javax.sound.sampled.*;
import javax.swing.*;
import java.awt.*;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.io.*;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.Map;
import java.util.Base64;
import java.net.URISyntaxException;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

public class AudioWebSocketServerWithPlot extends WebSocketServer {
    private final Map<String, UserAudioState> userAudioMap = new ConcurrentHashMap<>();
    private SourceDataLine speaker;
    private final SampleBuffer sampleBuffer;
    private final WaveformPanel waveformPanel;
    private FileOutputStream pcmDumpStream;
    private static final int USER_ID_LENGTH = 8;
    private static final int ROOM_ID_LENGTH = 4;
    
    // WAV recording
    private List<byte[]> wavBuffer;
    private boolean isRecording = false;
    private String currentRoomId = null;
    private String currentUserId = null;
    private static final int SAMPLE_RATE = 16000;
    private static final String TRANSCRIPTION_SERVER = "https://echoscreen.onrender.com";
    
    // Auto recording
    private ScheduledExecutorService autoRecordScheduler;
    private boolean autoRecordingEnabled = false;
    private static final int RECORDING_INTERVAL_SECONDS = 5;
    private JLabel statusLabel;
    private int clientCount = 0;
    private int cycleCount = 0;

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

        // setupSpeaker();
        setupUI();
    }

    // private void setupSpeaker() throws LineUnavailableException {
    //     AudioFormat format = new AudioFormat(
    //             SAMPLE_RATE,
    //             16,
    //             1,
    //             true,
    //             false
    //     );

    //     DataLine.Info info = new DataLine.Info(SourceDataLine.class, format);
    //     speaker = (SourceDataLine) AudioSystem.getLine(info);
    //     speaker.open(format);
    //     speaker.start();
    //     System.out.println("🎧 Speaker ready for PCM audio (16-bit @ " + SAMPLE_RATE + "Hz)");
    // }

    private void setupUI() {
        SwingUtilities.invokeLater(() -> {
            JFrame frame = new JFrame("ESP32 Audio Waveform — WebSocket Server");
            frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
            frame.setLayout(new BorderLayout());
            
            JPanel controlPanel = new JPanel();
            controlPanel.setLayout(new FlowLayout());
            
            statusLabel = new JLabel("⚪ Waiting for client connection...");
            statusLabel.setFont(new Font("Arial", Font.BOLD, 14));
            
            controlPanel.add(statusLabel);
            
            frame.add(controlPanel, BorderLayout.NORTH);
            frame.add(waveformPanel, BorderLayout.CENTER);
            frame.setSize(800, 350);
            frame.setLocationRelativeTo(null);
            frame.setVisible(true);

            // Swing Timer for UI updates (not recording)
            javax.swing.Timer uiTimer = new javax.swing.Timer(25, e -> waveformPanel.repaint());
            uiTimer.start();
        });
    }

    private void startAutoRecording() {
        if (autoRecordingEnabled) return;
        
        autoRecordingEnabled = true;
        cycleCount = 0;
        updateStatus("🔴 Auto-recording ACTIVE (5s intervals)");
        System.out.println("🤖 Auto-recording started (5 second intervals)");
        System.out.println("📋 Pattern: START → 5s → STOP/TRANSCRIBE → START → 5s → STOP/TRANSCRIBE...");
        
        autoRecordScheduler = Executors.newSingleThreadScheduledExecutor();
        
        // Start first recording immediately
        autoRecordScheduler.execute(() -> {
            cycleCount++;
            System.out.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
            System.out.println("🔄 CYCLE #" + cycleCount + " - RECORDING STARTED");
            System.out.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
            startRecording();
            updateStatus("🔴 Recording... (Cycle #" + cycleCount + ")");
        });
        
        // Schedule repeating task: Stop → Start every 5 seconds
        autoRecordScheduler.scheduleAtFixedRate(() -> {
            try {
                // Stop current recording and transcribe
                System.out.println("⏹️  Stopping recording cycle #" + cycleCount);
                updateStatus("⏸️  Stopping & Transcribing... (Cycle #" + cycleCount + ")");
                stopRecordingAndTranscribe();
                
                // Wait a bit for processing
                Thread.sleep(500);
                
                // Start next recording
                cycleCount++;
                System.out.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
                System.out.println("🔄 CYCLE #" + cycleCount + " - RECORDING STARTED");
                System.out.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
                startRecording();
                updateStatus("🔴 Recording... (Cycle #" + cycleCount + ")");
                
            } catch (Exception e) {
                System.err.println("❌ Error in auto-recording cycle: " + e.getMessage());
                e.printStackTrace();
            }
        }, RECORDING_INTERVAL_SECONDS, RECORDING_INTERVAL_SECONDS, TimeUnit.SECONDS);
    }

    private void stopAutoRecording() {
        if (!autoRecordingEnabled) return;
        
        autoRecordingEnabled = false;
        
        if (autoRecordScheduler != null) {
            autoRecordScheduler.shutdown();
            try {
                if (!autoRecordScheduler.awaitTermination(2, TimeUnit.SECONDS)) {
                    autoRecordScheduler.shutdownNow();
                }
            } catch (InterruptedException e) {
                autoRecordScheduler.shutdownNow();
            }
            autoRecordScheduler = null;
        }
        
        // Stop any ongoing recording
        if (isRecording) {
            System.out.println("⏹️  Final stop - completing last recording");
            stopRecordingAndTranscribe();
        }
        
        updateStatus("⚪ Auto-recording STOPPED (Completed " + cycleCount + " cycles)");
        System.out.println("\n🛑 Auto-recording stopped after " + cycleCount + " cycles");
        cycleCount = 0;
    }

    private void updateStatus(String message) {
        SwingUtilities.invokeLater(() -> {
            if (statusLabel != null) {
                statusLabel.setText(message);
            }
        });
    }

    private void startRecording() {
        synchronized (wavBuffer) {
            wavBuffer.clear();
        }
        isRecording = true;
        currentRoomId = null;
        currentUserId = null;
    }

    private void stopRecordingAndTranscribe() {
        if (!isRecording) {
            System.out.println("⚠️  No active recording to stop");
            return;
        }
        
        isRecording = false;
        
        new Thread(() -> {
            try {
                Thread.sleep(200);
                
                if (currentUserId == null || currentRoomId == null) {
                    System.err.println("❌ No userId or roomId captured during recording");
                    return;
                }
                
                // Check if there's actual audio data
                int totalAudioSize;
                synchronized (wavBuffer) {
                    totalAudioSize = wavBuffer.stream().mapToInt(a -> a.length).sum();
                }
                
                if (totalAudioSize == 0) {
                    System.out.println("⏭️  Skipping empty recording (no audio data)");
                    return;
                }
                
                String wavFilePath = "recording_" + currentUserId + "_" + System.currentTimeMillis() + ".wav";
                saveAsWav(wavFilePath);
                
                File wavFile = new File(wavFilePath);
                if (!wavFile.exists()) {
                    System.err.println("❌ WAV file doesn't exist");
                    return;
                }
                
                // Check if audio has sufficient energy (not silence)
                if (!hasSignificantAudio(wavFilePath)) {
                    System.out.println("🔇 Skipping silent audio - API request saved!");
                    wavFile.delete(); // Clean up silent recording
                    return;
                }
                
                System.out.println("✅ WAV saved: " + wavFilePath + " (" + wavFile.length() + " bytes)");
                System.out.println("🚀 Transcribing... [User: " + currentUserId + ", Room: " + currentRoomId + "]");
                
                sendToTranscriptionServer(wavFilePath, currentUserId, currentRoomId);
                //clean the wavFilePath file
                
            } catch (Exception e) {
                System.err.println("❌ Error during transcription: " + e.getMessage());
                e.printStackTrace();
            }
        }).start();
    }
    
    /**
     * Analyzes audio file to detect if it contains significant audio (speech/sound)
     * or if it's just silence/background noise
     * @return true if audio contains significant sound, false if mostly silent
     */
    private boolean hasSignificantAudio(String wavFilePath) {
        try {
            File file = new File(wavFilePath);
            FileInputStream fis = new FileInputStream(file);
            
            // Skip WAV header (44 bytes)
            fis.skip(44);
            
            byte[] buffer = new byte[4096];
            int bytesRead;
            long totalEnergy = 0;
            int sampleCount = 0;
            
            // // Thresholds for detection (adjustable)
            // double amplitudeThreshold = 500.0;  // Individual sample threshold
            int significantSampleCount = 0;
            double amplitudeThreshold = 3000.0;  // Was 500, now 6x higher

            // Read and analyze all samples
            while ((bytesRead = fis.read(buffer)) != -1) {
                for (int i = 0; i < bytesRead - 1; i += 2) {
                    // Read 16-bit PCM sample (little-endian)
                    short sample = (short) ((buffer[i] & 0xFF) | ((buffer[i + 1] & 0xFF) << 8));
                    double amplitude = Math.abs(sample);
                    
                    totalEnergy += amplitude;
                    sampleCount++;
                    
                    // Count samples above threshold
                    if (amplitude > amplitudeThreshold) {
                        significantSampleCount++;
                    }
                }
            }
            fis.close();
            
            if (sampleCount == 0) return false;
            
            // Calculate metrics
            double averageEnergy = (double) totalEnergy / sampleCount;
            double significantRatio = (double) significantSampleCount / sampleCount;
            
            // Decision criteria:
            // Audio is considered significant if EITHER:
            // 1. Average energy is above 100 (general sound level)
            // 2. At least 1% of samples are loud (sporadic speech/sounds)
            // boolean hasEnergy = averageEnergy > 100.0 || significantRatio > 0.01;
            boolean hasEnergy = averageEnergy > 1000.0 || significantRatio > 0.05;
            // Log analysis results
            System.out.println("📊 Audio Analysis:");
            System.out.println("   - Average energy: " + String.format("%.2f", averageEnergy));
            System.out.println("   - Significant samples: " + String.format("%.2f%%", significantRatio * 100));
            System.out.println("   - Decision: " + (hasEnergy ? "✅ SEND (has speech)" : "❌ SKIP (silence)"));
            
            return hasEnergy;
            
        } catch (Exception e) {
            System.err.println("⚠️  Error analyzing audio, sending anyway to be safe: " + e.getMessage());
            return true; // If analysis fails, send to API to avoid missing real speech
        }
    }

    private void saveAsWav(String filename) throws IOException {
        synchronized (wavBuffer) {
            int totalSize = 0;
            for (byte[] chunk : wavBuffer) {
                totalSize += chunk.length;
            }

            if (totalSize == 0) {
                throw new IOException("No audio data recorded");
            }

            FileOutputStream fos = new FileOutputStream(filename);
            writeWavHeader(fos, totalSize, SAMPLE_RATE, 1, 16);
            
            for (byte[] chunk : wavBuffer) {
                fos.write(chunk);
            }
            
            fos.close();
        }
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

    private void sendToTranscriptionServer(String wavFilePath, String userId, String roomId) {
        try {
            File file = new File(wavFilePath);
            byte[] fileData = new byte[(int) file.length()];
            
            try (FileInputStream fis = new FileInputStream(file)) {
                fis.read(fileData);
            }
            
            String base64Audio = Base64.getEncoder().encodeToString(fileData);
            
            IO.Options options = new IO.Options();
            options.forceNew = true;
            options.reconnection = true;
            
            Socket socket = IO.socket(TRANSCRIPTION_SERVER, options);
            
            socket.on(Socket.EVENT_CONNECT, args -> {
                System.out.println("🔗 Connected to transcription server");
                
                try {
                    JSONObject data = new JSONObject();
                    data.put("userId", userId.trim());
                    data.put("roomId", roomId.trim());
                    data.put("audioFile", base64Audio);
                    
                    socket.emit("audioTranscription", data);
                    
                } catch (Exception e) {
                    System.err.println("❌ Error creating data: " + e.getMessage());
                    e.printStackTrace();
                    socket.disconnect();
                }
            });
            
            socket.on("audioTranscription", args -> {
                System.out.println("\n📝 TRANSCRIPTION RESULT:");
                System.out.println("═══════════════════════════════════════");
                System.out.println(args[0].toString());
                System.out.println("═══════════════════════════════════════\n");
                socket.disconnect();
            });
            
            socket.on("error", args -> {
                System.err.println("❌ Socket.IO Error: " + args[0]);
                socket.disconnect();
            });
            
            socket.on(Socket.EVENT_CONNECT_ERROR, args -> {
                System.err.println("❌ Connection Error: " + args[0]);
            });
            
            socket.on(Socket.EVENT_DISCONNECT, args -> {
                System.out.println("🔌 Disconnected from transcription server");
            });
            
            socket.connect();
            
        } catch (URISyntaxException e) {
            System.err.println("❌ Invalid server URL: " + e.getMessage());
            e.printStackTrace();
        } catch (Exception e) {
            System.err.println("❌ Error sending transcription request: " + e.getMessage());
            e.printStackTrace();
        }
    }

    @Override
    public void onOpen(WebSocket conn, ClientHandshake handshake) {
        clientCount++;
        System.out.println("✅ Client connected: " + conn.getRemoteSocketAddress());
        System.out.println("👥 Active clients: " + clientCount);
        
        // Start auto-recording when first client connects
        if (clientCount == 1) {
            startAutoRecording();
        }
    }

    @Override
    public void onClose(WebSocket conn, int code, String reason, boolean remote) {
        clientCount--;
        System.out.println("❌ Client disconnected: " + conn.getRemoteSocketAddress());
        System.out.println("👥 Active clients: " + clientCount);
        
        // Stop auto-recording when last client disconnects
        if (clientCount == 0) {
            stopAutoRecording();
        }
    }

    @Override
    public void onMessage(WebSocket conn, String message) {
        // Ignore text messages
    }

    @Override
    public void onMessage(WebSocket conn, ByteBuffer message) {
        byte[] fullData = message.array();
        int expectedMinLength = USER_ID_LENGTH + ROOM_ID_LENGTH + 4;
        
        if (fullData.length < expectedMinLength) {
            return;
        }

        String userId = new String(fullData, 0, USER_ID_LENGTH).trim();
        String roomId = new String(fullData, USER_ID_LENGTH, ROOM_ID_LENGTH).trim();
        
        if (isRecording && currentUserId == null) {
            currentUserId = userId;
            currentRoomId = roomId;
            System.out.println("📍 Captured - UserId: " + userId + ", RoomId: " + roomId);
        }

        int pcmOffset = USER_ID_LENGTH + ROOM_ID_LENGTH;
        byte[] pcm32Bytes = new byte[fullData.length - pcmOffset];
        System.arraycopy(fullData, pcmOffset, pcm32Bytes, 0, pcm32Bytes.length);

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

        if (isRecording) {
            synchronized (wavBuffer) {
                wavBuffer.add(pcmBytes.clone());
            }
        }

        userAudioMap.computeIfAbsent(userId, id -> new UserAudioState(id, roomId))
                    .addAudioChunk(pcmBytes);

        try {
            if (pcmDumpStream != null) pcmDumpStream.write(pcmBytes);
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
        System.out.println("💡 Auto-recording will start when a client connects");
        System.out.println("📋 Cycle: START → Record 5s → STOP/TRANSCRIBE → START → ...");
        System.out.println("🔇 Silence detection ENABLED - saves API requests!");
        System.out.println("🔗 Transcription server: " + TRANSCRIPTION_SERVER);
    }

    public static void main(String[] args) throws Exception {
        int port = 8081;
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

    // ===== UserAudioState =====
    static class UserAudioState {
        public List<byte[]> wavBuffer = new ArrayList<>();
        public boolean isRecording = true;
        public String userId;
        public String roomId;
        public String wavFilePath;

        public UserAudioState(String userId, String roomId) {
            this.userId = userId;
            this.roomId = roomId;
            this.wavFilePath = "user_" + userId + "_room_" + roomId + "_" + System.currentTimeMillis() + ".wav";
        }

        public void addAudioChunk(byte[] chunk) {
            if (isRecording) wavBuffer.add(chunk.clone());
        }

        public void stopAndSave() throws IOException {
            isRecording = false;
            int totalSize = wavBuffer.stream().mapToInt(a -> a.length).sum();
            try (FileOutputStream fos = new FileOutputStream(wavFilePath)) {
                writeWavHeader(fos, totalSize, SAMPLE_RATE, 1, 16);
                for (byte[] chunk : wavBuffer) fos.write(chunk);
            }
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
            return new byte[]{
                    (byte) (value & 0xFF),
                    (byte) ((value >> 8) & 0xFF),
                    (byte) ((value >> 16) & 0xFF),
                    (byte) ((value >> 24) & 0xFF)
            };
        }

        private byte[] shortToBytes(short value) {
            return new byte[]{
                    (byte) (value & 0xFF),
                    (byte) ((value >> 8) & 0xFF)
            };
        }
    }
}