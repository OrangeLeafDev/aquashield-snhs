//#include <SoftwareSerial.begin>
#include <SoftwareSerial.h>

// UART
const int TIMEOUT = 1000; // Timeout to send data in MS
const int RX = 2; // Uno RX to ESP32 TX2 via voltage dividor
const int TX = 3; // Uno TX to ESP32 RX2
SoftwareSerial espSerial(RX, TX);
String uartBuffer;
// Ultrasonic
const int trigPin = 9;
const int echoPin = 10;
float duration, distance, water;
// L298N
const int motorIN1 = 5;
const int motorIN2 = 6;
const int motorEN = 9;
bool motorReverse = false; // set to true to flip the direction
bool motorRunning = false;

void setMotor(bool on) {
  digitalWrite(motorIN1, on != motorReverse);
  digitalWrite(motorIN2, on == motorReverse);
  digitalWrite(motorEN, on);
}

// Code timers
float lastWater = 0;
float lastUltrasonic = 0;
float lastPost = 0;
float lastMotor = 0;

float readDistance() {
  // Timing gimmicks, idk
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  unsigned long duration = pulseIn(echoPin, HIGH, 25000);
  if (duration == 0)
    return -1;
  return duration * 0.0343 / 2.0; // Returns data in cm via approximate conversion
}

void processUART(String command) {
  command.trim();
  if (command == "CHECK") {
    espSerial.println("ONLINE");
  }
}

void uartIO() {
  while (espSerial.available()) {
    char c = espSerial.read();
    if (c == '\n') {
      processUART(uartBuffer);
      uartBuffer = "";
    }
    else if (c != '\r') {
      uartBuffer += c;
    }
  }
}

void setup() {
  // Serial debugging setup
  Serial.begin(9600);

  // UART for ESP32 setup
  espSerial.begin(9600);
  
  // Ultrasonic Setup
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.println("Arduino Uno Ready. Type a message below to send to ESP32:");
}

void loop() {
  uartIO();
  // Water check : 20ms
  if (millis() - lastWater >= 20) {
    lastWater = millis();
    water = analogRead(A0);
  }
  // Ultrasonic check : 60ms
  if (millis() - lastUltrasonic >= 60) {
    lastUltrasonic = millis();
    distance = readDistance();
  }
  // Motor check : 50ms
  if (millis() - lastMotor >= 50) {
    lastMotor = millis();
    setMotor(motorRunning);
  }
  // Post
  if (millis() - lastPost >= TIMEOUT) {
    String json_data = "{";
    json_data += "\"distance\":" + String(distance, 1) + ",";
    json_data += "\"water\":" + String(water) + ",";
    json_data += "\"motor\":" + String(motorRunning ? 1 : 0) + ",";
    espSerial.print(json_data);
  }
}
