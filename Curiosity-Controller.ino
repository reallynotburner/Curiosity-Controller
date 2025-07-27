/*********
  Curiosity Controller
  Drivable scale model of the Mars Science Laboratory, AKA, "Curiosity"
  Based on code examples by:
  Rui Santos & Sara Santos - Random Nerd Tutorials: 
  https://RandomNerdTutorials.com/esp32-websocket-server-sensor/
  Frank Poth:
  https://github.com/pothonprogramming/pothonprogramming.github.io/tree/master/content/touch-controller

  reallynotburner@gmail.com
  Joshua Brown 2025 

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <ESP32Servo.h>
#include <Preferences.h>

// Replace with your network credentials
const char *ssid = "curiosity";
const char *password = "callyourmom";

// Motor Driver pins
const int PWM01 = 26;  // 16 corresponds to GPIO 16
const int AN1 = 33;
const int AN2 = 25;
const int STBY = 27;

// setting motor PWM properties
const int freq = 5000;
const int resolution = 8;

// Store preferences, like steering calibration
Preferences preferences;
// uS +/- of center to add to the incoming controls
short steerCal01 = 0; // calibration from preferences
short currentSteerCal01 = 0;  // calibration in RAM during currnet calibration session
char *steerKey = "steer01"; // the name of the calibration value for storage and retrieval

// steering pins
const int STEER01 = 19;
Servo steer01;

void initMotor() {
  // configure motor PWM
  pinMode(PWM01, OUTPUT);

  // set the control pins as outputs
  pinMode(AN1, OUTPUT);
  pinMode(AN2, OUTPUT);
  pinMode(STBY, OUTPUT);

  digitalWrite(STBY, HIGH);
  digitalWrite(AN1, LOW);
  digitalWrite(AN2, LOW);
}

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// Json Variable to Hold Sensor Readings
JsonDocument readings;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;


// Get Sensor Readings and return JSON object
String getSensorReadings() {
  readings["temperature"] = "Hot!";
  readings["humidity"] = "Moist!";
  readings["pressure"] = "Vacuous!";
  char serializedReadings[256];
  serializeJson(readings, serializedReadings);
  return serializedReadings;
}

// Initialize LittleFS
void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}


void notifyClients(String sensorReadings) {
  ws.textAll(sensorReadings);
}

void updateMotors(String message) {
  JsonDocument messageObject;
  deserializeJson(messageObject, message);

  float horizontal = messageObject["horizontal"];
  float vertical = messageObject["vertical"];

  float mappedVertical = 0;

  // find direction and speed of motors
  if (vertical > 0.0) {
    mappedVertical = vertical*255.0;
    digitalWrite(AN1, HIGH);
    digitalWrite(AN2, LOW);
    analogWrite(PWM01, (unsigned int) mappedVertical);
    Serial.print("forward at: ");
    Serial.println((unsigned int) mappedVertical);
  } else if (vertical < 0.0) {
    mappedVertical = -vertical*255.0;
    digitalWrite(AN1, LOW);
    digitalWrite(AN2, HIGH);
    analogWrite(PWM01, (unsigned int) mappedVertical);
    Serial.print("backward at: ");
    Serial.println((unsigned int) mappedVertical);
  } else {
    Serial.println("Zero Motors!");
    digitalWrite(AN1, LOW);
    digitalWrite(AN2, LOW);
    analogWrite(PWM01, 0);
    Serial.println("nowhere at: 0");
  }

  // find steering values
  // steer01.writeMicroseconds(1500); // middle
  // steer01.writeMicroseconds(1000); // beginning
  // steer01.writeMicroseconds(2000); // end
  steer01.writeMicroseconds((int)(500.0 * horizontal) + 1500);
  Serial.print("steer01 uS: ");
  Serial.println((int)(500.0 * horizontal) + 1500);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    // How to ingest the incoming data from web clients, without weird symbols and overflow values:
    String rawData = (char *)data;
    String message = rawData.substring(0, len);
    Serial.println(message);
    updateMotors(message);
    String sensorReadings = getSensorReadings();
    notifyClients(sensorReadings);
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void storeValue (short value, char *key) {
  preferences.begin("preferences", false);
  short currentVal = preferences.getShort(key, 0);

  Serial.print(key);
  Serial.print(" current value: ");
  Serial.println(currentVal);

  preferences.putShort(key, value);
  currentVal = preferences.getShort(key, 0);
  preferences.end();

  Serial.print(key);
  Serial.print(" updated value is: ");
  Serial.println(currentVal);
}

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);

  initLittleFS();
  initWebSocket();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  // Start server
  server.begin();

  if (!MDNS.begin("controller")) {
    Serial.println("Error setting up mDNS");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  // Setup Motors
  initMotor();
  
  steer01.attach(STEER01);

  delay(5000);
  storeValue(799, steerKey);
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    String sensorReadings = getSensorReadings();
    // Serial.print(sensorReadings);
    notifyClients(sensorReadings);
    lastTime = millis();

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
  }
  ws.cleanupClients();
}
