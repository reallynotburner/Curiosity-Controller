#ifndef CURIOSITY_HEADER // Replace MY_HEADER_H with a unique identifier, usually based on the filename
#define CURIOSITY_HEADER

#include <ArduinoJson.h>
#include <Preferences.h>

// Store preferences, like steering calibration
Preferences preferences;

// Replace with your network credentials
const char *ssid = "curiosity";
const char *password = "marsrover";

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

// uS +/- of center to add to the incoming controls
short steerCal01 = 0;          // calibration from preferences
short steerCal02 = 0;
short steerCal05 = 0;
short steerCal06 = 0;
char *steerKey01 = "steer01";  // the name of the calibration value for storage and retrieval
char *steerKey02 = "steer02";
char *steerKey05 = "steer05";
char *steerKey06 = "steer06";

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

// 182.65 reading units per volt.
// 5.20 volts as 5.193 volts 
const int supplySense = 32;
float voltageRatio = 182.7;

float getVoltage(int pin) {
  int voltageValue = analogReadMilliVolts(pin);
  float voltageConverted = voltageValue / voltageRatio;
  // Serial.print("Voltage Sensed: ");
  // Serial.print(voltageConverted);
  // Serial.println(" volts");
  return voltageConverted;
}

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
  float supplyVoltage = getVoltage(supplySense);
  sensorReadings["supplyVoltage"] = supplyVoltage;
  sensorReadings["cal01"] = steerCal01;
  sensorReadings["cal02"] = steerCal02;
  sensorReadings["cal05"] = steerCal05;
  sensorReadings["cal06"] = steerCal06;
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
    unsigned int mappedHorizontal = (unsigned int) abs(horizontal * 64.0);
    digitalWrite(AN1, HIGH);
    digitalWrite(AN2, LOW);
    digitalWrite(BN1, HIGH);
    digitalWrite(BN2, LOW);
    if (horizontal > 0) {
      analogWrite(PWM01, mappedVertical - mappedHorizontal);
      analogWrite(PWM02, mappedVertical);
    } else {
      analogWrite(PWM01, mappedVertical);
      analogWrite(PWM02, mappedVertical - mappedHorizontal);
    }
    // Serial.print("forward: ");
    // Serial.println(mappedVertical);
}

void backward(float vertical, float horizontal) {
    unsigned int mappedVertical = (unsigned int) abs(vertical * 255.0);
    unsigned int mappedHorizontal = (unsigned int) abs(horizontal * 64.0);
    digitalWrite(AN1, LOW);
    digitalWrite(AN2, HIGH);
    digitalWrite(BN1, LOW);
    digitalWrite(BN2, HIGH);
    if (horizontal > 0) {
      analogWrite(PWM01, mappedVertical - mappedHorizontal);
      analogWrite(PWM02, mappedVertical);
    } else {
      analogWrite(PWM01, mappedVertical);
      analogWrite(PWM02, mappedVertical - mappedHorizontal);
    }
    // Serial.print("backward: ");
    // Serial.println(mappedVertical);
}

void spin(float horizontal) {    
    int spinLocation = 500;
    // Toe-in all the steering servoes
    steer01.writeMicroseconds(steerCal01 - spinLocation + 1500);
    steer02.writeMicroseconds(steerCal02 - spinLocation + 1500);
    steer05.writeMicroseconds(steerCal05 + spinLocation + 1500);
    steer06.writeMicroseconds(steerCal06 - spinLocation + 1500);

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
      // Serial.print("spin left: ");
      // Serial.println(mappedHorizontal);
    }
}

void stop() {
    digitalWrite(AN1, LOW);
    digitalWrite(AN2, LOW);
    digitalWrite(BN1, LOW);
    digitalWrite(BN2, LOW);
    analogWrite(PWM01, 0);
    analogWrite(PWM02, 0);
    // Serial.println("STOPPED");
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

void initSteering() {
  steerCal01 = getStoredValue(steerKey01);
  steerCal02 = getStoredValue(steerKey02);
  steerCal05 = getStoredValue(steerKey05);
  steerCal06 = getStoredValue(steerKey06);
  
  steer01.attach(STEER01);
  steer02.attach(STEER02);
  steer05.attach(STEER05);
  steer06.attach(STEER06);
  
  // move steering servos to middle position
  steer01.writeMicroseconds(steerCal01 + 1500);
  steer02.writeMicroseconds(steerCal02 + 1500);
  steer05.writeMicroseconds(steerCal05 + 1500);
  steer06.writeMicroseconds(steerCal06 + 1500);
}

#endif // CURIOSITY_HEADER