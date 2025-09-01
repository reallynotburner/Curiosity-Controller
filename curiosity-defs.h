#ifndef CURIOSITY_HEADER // Replace MY_HEADER_H with a unique identifier, usually based on the filename
#define CURIOSITY_HEADER

#include <ArduinoJson.h>

// Replace with your network credentials
const char *ssid = "curiosity";
const char *password = "callyourmom";

// Motor Driver pins
const int PWM01 = 26;
const int AN1 = 33;
const int AN2 = 25;
const int STBY = 27;
const int PWM02 = 16;
const int BN1 = 4;
const int BN2 = 2;

// setting motor PWM properties
const int freq = 5000;
const int resolution = 8;

// steering pins
const int STEER01 = 19;
const int STEER02 = 17;
const int STEER05 = 18;
const int STEER06 = 5;
Servo steer01;
Servo steer02;
Servo steer05;
Servo steer06;


// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

void initMotor() {
  // configure motor PWM
  pinMode(PWM01, OUTPUT);
  pinMode(PWM02, OUTPUT);

  // set the control pins as outputs
  pinMode(AN1, OUTPUT);
  pinMode(AN2, OUTPUT);
  pinMode(BN1, OUTPUT);
  pinMode(BN2, OUTPUT);
  pinMode(STBY, OUTPUT);

  digitalWrite(STBY, HIGH);
  digitalWrite(AN1, LOW);
  digitalWrite(AN2, LOW);
  digitalWrite(BN1, LOW);
  digitalWrite(BN2, LOW);
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

void forward(float vertical, float horizontal) {
    unsigned int mappedVertical = (unsigned int) abs(vertical * 255.0);
    digitalWrite(AN1, HIGH);
    digitalWrite(AN2, LOW);
    analogWrite(PWM01, mappedVertical);
    digitalWrite(BN1, HIGH);
    digitalWrite(BN2, LOW);
    analogWrite(PWM02, mappedVertical);
    Serial.print("forward: ");
    Serial.println(mappedVertical);
}

void backward(float vertical, float horizontal) {
    unsigned int mappedVertical = (unsigned int) abs(vertical * 255.0);
    digitalWrite(AN1, LOW);
    digitalWrite(AN2, HIGH);
    analogWrite(PWM01, mappedVertical);
    digitalWrite(BN1, LOW);
    digitalWrite(BN2, HIGH);
    analogWrite(PWM02, mappedVertical);
    Serial.print("backward: ");
    Serial.println(mappedVertical);
}

void spin(float horizontal) {
    unsigned int mappedHorizontal = (unsigned int) abs(horizontal * 255.0);
    if (horizontal > 0) {
      digitalWrite(AN1, HIGH);
      digitalWrite(AN2, LOW);
      analogWrite(PWM01, mappedHorizontal);
      digitalWrite(BN1, LOW);
      digitalWrite(BN2, HIGH);
      analogWrite(PWM02, mappedHorizontal);
      Serial.print("spin right: ");
      Serial.println(mappedHorizontal);
    } else {
      digitalWrite(AN1, LOW);
      digitalWrite(AN2, HIGH);
      analogWrite(PWM01, mappedHorizontal);
      digitalWrite(BN1, HIGH);
      digitalWrite(BN2, LOW);
      analogWrite(PWM02, mappedHorizontal);
      Serial.print("spin left: ");
      Serial.println(mappedHorizontal);
    }
}

void stop() {
    digitalWrite(AN1, LOW);
    digitalWrite(AN2, LOW);
    digitalWrite(BN1, LOW);
    digitalWrite(BN2, LOW);
    analogWrite(PWM01, 0);
    analogWrite(PWM02, 0);
    Serial.println("STOPPED");
}

#endif // CURIOSITY_HEADER