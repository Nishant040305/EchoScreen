
@REM @REM if not exist AudioWebSocketServerWithPlot.class (
@REM @REM     echo Compiling AudioWebSocketServerWithPlot.java...
@REM @REM     javac -cp .;Java-WebSocket-1.5.4.jar;slf4j-api-2.0.9.jar;concentus-1.0.1.jar;slf4j-simple-2.0.9.jar AudioWebSocketServerWithPlot.java
@REM @REM ) else (
@REM @REM     echo Already compiled, skipping...
@REM @REM )
@REM echo Running server...
@REM @echo off
@REM javac -cp .;Java-WebSocket-1.5.4.jar;slf4j-api-2.0.9.jar;concentus-1.0.1.jar;slf4j-simple-2.0.9.jar AudioWebSocketServerWithPlot.java
@REM java -cp .;Java-WebSocket-1.5.4.jar;concentus-1.0.1.jar;slf4j-api-2.0.9.jar;slf4j-simple-2.0.9.jar AudioWebSocketServerWithPlot
@echo off
echo Compiling AudioWebSocketServerWithPlot.java...
javac -cp ".;Java-WebSocket-1.5.4.jar;slf4j-api-2.0.9.jar;concentus-1.0.1.jar;slf4j-simple-2.0.9.jar;socket.io-client-2.1.0.jar;engine.io-client-2.1.0.jar;json-20180813.jar;okhttp-3.12.12.jar;okio-1.17.5.jar" AudioWebSocketServerWithPlot.java

echo Running server...
java -cp ".;Java-WebSocket-1.5.4.jar;slf4j-api-2.0.9.jar;concentus-1.0.1.jar;slf4j-simple-2.0.9.jar;socket.io-client-2.1.0.jar;engine.io-client-2.1.0.jar;json-20180813.jar;okhttp-3.12.12.jar;okio-1.17.5.jar" AudioWebSocketServerWithPlot
