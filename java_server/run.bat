
@REM if not exist AudioWebSocketServerWithPlot.class (
@REM     echo Compiling AudioWebSocketServerWithPlot.java...
@REM     javac -cp .;Java-WebSocket-1.5.4.jar;slf4j-api-2.0.9.jar;concentus-1.0.1.jar;slf4j-simple-2.0.9.jar AudioWebSocketServerWithPlot.java
@REM ) else (
@REM     echo Already compiled, skipping...
@REM )
echo Running server...
@echo off
javac -cp .;Java-WebSocket-1.5.4.jar;slf4j-api-2.0.9.jar;concentus-1.0.1.jar;slf4j-simple-2.0.9.jar AudioWebSocketServerWithPlot.java
java -cp .;Java-WebSocket-1.5.4.jar;concentus-1.0.1.jar;slf4j-api-2.0.9.jar;slf4j-simple-2.0.9.jar AudioWebSocketServerWithPlot
