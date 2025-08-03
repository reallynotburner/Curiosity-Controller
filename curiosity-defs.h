#ifndef CURIOSITY_HEADER // Replace MY_HEADER_H with a unique identifier, usually based on the filename
#define CURIOSITY_HEADER

#include <ArduinoJson.h>

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

// steering pins
const int STEER01 = 19;
Servo steer01;


// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

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

// Get Sensor Readings and return JSON object
String getSensorReadings() {
  JsonDocument sensorReadings;
  sensorReadings["temperature"] = "Hot!";
  sensorReadings["humidity"] = "Moist!";
  sensorReadings["pressure"] = "Vacuous!";
  char serializedReadings[256];
  serializeJson(sensorReadings, serializedReadings);
  return serializedReadings;
}

// Initialize LittleFS
void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

#endif // CURIOSITY_HEADER