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
#include <curiosity-defs.h>

// Store preferences, like steering calibration
Preferences preferences;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

bool calibrating = false;
unsigned short calibrationAxis = 0;

// uS +/- of center to add to the incoming controls
short steerCal01 = 0;          // calibration from preferences
short steerCal02 = 0;
short steerCal05 = 0;
short steerCal06 = 0;
char *steerKey01 = "steer01";  // the name of the calibration value for storage and retrieval
char *steerKey02 = "steer02";
char *steerKey05 = "steer05";
char *steerKey06 = "steer06";

void notifyClients(String sensorReadings) {
  ws.textAll(sensorReadings);
}

void storeValue (char *key, short value) {
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

short getStoredValue (char *key) {
  preferences.begin("preferences", false);
  short currentVal = preferences.getShort(key, 0);

  Serial.print("retrieving current value: ");
  Serial.print(key);
  Serial.print(": ");
  Serial.println(currentVal);
  return currentVal;
}

void steer (float horizontal) {
  // find steering values
  int diff = 0;
  int value01 = 0;
  int value02 = 0;
  int value05 = 0;
  int value06 = 0;
  if (!calibrationAxis) {  // regular steering, control all motors
    diff = (int)(500.0 * horizontal);
    value01 = diff + steerCal01 + 1500;
    value02 = steerCal02 - diff + 1500;
    value05 = steerCal05 - diff + 1500;
    value06 = diff + steerCal06 + 1500;
    steer01.writeMicroseconds(value01);
    steer02.writeMicroseconds(value02);
    steer05.writeMicroseconds(value05);
    steer06.writeMicroseconds(value06);
    // TODO: add the other steering motors
  } else {  // calibration mode, only do one motor at a time
    diff = (int)(50.0 * horizontal);
    switch (calibrationAxis) {
      case 0:  // no axes are being calibrated
        break;
      case 1:  // calibrate steering servo 1
        value01 = diff + steerCal01 + 1500;
        steer01.writeMicroseconds(value01);
        if (!calibrating) {  // hopefully only happens infrequently
          steerCal01 = steerCal01 + diff;
          storeValue(steerKey01, steerCal01);
        }
        break;
      case 2:  // calibrate steering servo 2
        value02 = diff + steerCal02 + 1500;
        steer02.writeMicroseconds(value02);
        if (!calibrating) {  // hopefully only happens infrequently
          steerCal02 = steerCal02 + diff;
          storeValue(steerKey02, steerCal02);
        }
        break;
      case 5:  // calibrate steering servo 5
        value05 = diff + steerCal05 + 1500;
        steer05.writeMicroseconds(value05);
        if (!calibrating) {  // hopefully only happens infrequently
          steerCal05 = steerCal05 + diff;
          storeValue(steerKey05, steerCal05);
        }
        break;
      case 6:  // calibrate steering servo 6
        value06 = diff + steerCal06 + 1500;
        steer06.writeMicroseconds(value06);
        if (!calibrating) {  // hopefully only happens infrequently
          steerCal06 = steerCal06 + diff;
          storeValue(steerKey06, steerCal06);
        }
        break;
      default:
        break;
    }
  }
}

void updateMotors(String message) {
  float mappedVertical = 0;

  JsonDocument messageObject;
  deserializeJson(messageObject, message);

  float horizontal = messageObject["horizontal"];
  float vertical = messageObject["vertical"];
  calibrationAxis = messageObject["calibrationAxis"];
  calibrating = messageObject["calibrating"];

  // find direction and speed of motors
  if (vertical > 0.0 && !calibrationAxis) {
    mappedVertical = vertical * 255.0;
    digitalWrite(AN1, HIGH);
    digitalWrite(AN2, LOW);
    analogWrite(PWM01, (unsigned int) mappedVertical);
    digitalWrite(BN1, HIGH);
    digitalWrite(BN2, LOW);
    analogWrite(PWM02, (unsigned int) mappedVertical);
    // Serial.print("forward at: ");
    // Serial.println((unsigned int) mappedVertical);
  } else if (vertical < 0.0 && !calibrationAxis) {
    mappedVertical = -vertical * 255.0;
    digitalWrite(AN1, LOW);
    digitalWrite(AN2, HIGH);
    analogWrite(PWM01, (unsigned int) mappedVertical);
    digitalWrite(BN1, LOW);
    digitalWrite(BN2, HIGH);
    analogWrite(PWM02, (unsigned int) mappedVertical);
    // Serial.print("backward at: ");
    // Serial.println((unsigned int) mappedVertical);
  } else {
    // Serial.println("Zero Motors!");
    digitalWrite(AN1, LOW);
    digitalWrite(AN2, LOW);
    digitalWrite(BN1, LOW);
    digitalWrite(BN2, LOW);
    analogWrite(PWM01, 0);
    analogWrite(PWM02, 0);
  }

  steer(horizontal);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    // How to ingest the incoming data from web clients, without weird symbols and overflow values:
    String rawData = (char *)data;
    String message = rawData.substring(0, len);
    // Serial.println(message);
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
  server.begin();

  if (!MDNS.begin("controller")) { // app is at http://controller.local
    Serial.println("Error setting up mDNS");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  initMotor();
  steerCal01 = getStoredValue(steerKey01);
  steerCal02 = getStoredValue(steerKey02);
  steerCal05 = getStoredValue(steerKey05);
  steerCal06 = getStoredValue(steerKey06);
  steer01.attach(STEER01);
  steer01.attach(STEER02);
  steer05.attach(STEER05);
  steer06.attach(STEER06);
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    String sensorReadings = getSensorReadings();
    // Serial.print(sensorReadings);
    notifyClients(sensorReadings);
    lastTime = millis();
  }
  ws.cleanupClients();
}
