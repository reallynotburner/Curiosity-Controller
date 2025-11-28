

// APA102 shift-array Leds to show status
typedef struct colorRGBB {
  uint8_t red, green, blue, brightness;
} colorRGBB;

#define LEDCLOCKPIN 22   // GPIO-PIN
#define LEDDATAPIN 23    // GPIO-PIN
#define LEDARRAYSIZE 3  // Number of APA102 LEDs in String
colorRGBB LED_STATE[LEDARRAYSIZE];

void ledWriteByte(uint8_t b) {
  uint8_t pos;
  for (pos = 0; pos <= 7; pos++) {
    digitalWrite(LEDDATAPIN, b >> (7 - pos) & 1);
    digitalWrite(LEDCLOCKPIN, HIGH);
    digitalWrite(LEDCLOCKPIN, LOW);
  }
}

void ledStartFrame() {
  ledWriteByte(0);
  ledWriteByte(0);
  ledWriteByte(0);
  ledWriteByte(0);
}

void ledEndFrame(uint16_t count) {
  ledWriteByte(0xFF);
  ledWriteByte(0xFF);
  ledWriteByte(0xFF);
  ledWriteByte(0xFF);
}

void ledWriteRGB(uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness) {
  ledWriteByte(0b11100000 | brightness);
  ledWriteByte(blue);
  ledWriteByte(green);
  ledWriteByte(red);
}

void ledWriteColor(colorRGBB color) {
  ledWriteRGB(color.red, color.green, color.blue, color.brightness);
}

void ledWriteColors(colorRGBB* colors, uint16_t count) {
  ledStartFrame();
  for (uint16_t i = 0; i < count; i++) {
    ledWriteColor(colors[i]);
  }
  ledEndFrame(count);
}

void redOn () {
  for (uint8_t led = 0; led < LEDARRAYSIZE; led++) {
    LED_STATE[led].red = 128;
    LED_STATE[led].green = 0;
    LED_STATE[led].blue = 0;
    LED_STATE[led].brightness = 10;
  }
  ledWriteColors(LED_STATE, LEDARRAYSIZE);
}

void greenOn () {
  for (uint8_t led = 0; led < LEDARRAYSIZE; led++) {
    LED_STATE[led].red = 0;
    LED_STATE[led].green = 128;
    LED_STATE[led].blue = 0;
    LED_STATE[led].brightness = 10;
  }
  ledWriteColors(LED_STATE, LEDARRAYSIZE);
}

void blueOn () {
  for (uint8_t led = 0; led < LEDARRAYSIZE; led++) {
    LED_STATE[led].red = 0;
    LED_STATE[led].green = 0;
    LED_STATE[led].blue = 128;
    LED_STATE[led].brightness = 10;
  }
  ledWriteColors(LED_STATE, LEDARRAYSIZE);
}

void yellowOn () {
  for (uint8_t led = 0; led < LEDARRAYSIZE; led++) {
    LED_STATE[led].red = 128;
    LED_STATE[led].green = 128;
    LED_STATE[led].blue = 0;
    LED_STATE[led].brightness = 10;
  }
  ledWriteColors(LED_STATE, LEDARRAYSIZE);
}

void salmonOn () {
  for (uint8_t led = 0; led < LEDARRAYSIZE; led++) {
    LED_STATE[led].red = 64;
    LED_STATE[led].green = 16;
    LED_STATE[led].blue = 13;
    LED_STATE[led].brightness = 10;
  }
  ledWriteColors(LED_STATE, LEDARRAYSIZE);  
}

void allLedsOff () {
  for (uint8_t led = 0; led < LEDARRAYSIZE; led++) {
    LED_STATE[led].red = 0;
    LED_STATE[led].green = 0;
    LED_STATE[led].blue = 0;
    LED_STATE[led].brightness = 0;
  }
  ledWriteColors(LED_STATE, LEDARRAYSIZE);
}

void initLedArray() {
  digitalWrite(LEDDATAPIN, LOW);
  pinMode(LEDDATAPIN, OUTPUT);
  digitalWrite(LEDCLOCKPIN, LOW);
  pinMode(LEDCLOCKPIN, OUTPUT);
  allLedsOff();
}

void ledCriticalAlert () {
  redOn();
  delay(5);
  allLedsOff();
  delay(5);
  redOn();
  delay(5);
  allLedsOff();
  delay(5);
  redOn();
  delay(5);
  allLedsOff();
}

void ledCautionAlert() {
  yellowOn();
  delay(5);
  allLedsOff();
  delay(5);
  yellowOn();
  delay(5);
  allLedsOff();
  delay(5);
  yellowOn();
  delay(5);
  allLedsOff();
}

void ledReceiveAlert() {
  salmonOn();
  delay(10);
  allLedsOff();
}
