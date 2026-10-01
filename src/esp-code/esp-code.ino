#include <WiFi.h>
#include <WebServer.h>

// Replace SSID and pw with the AP you want it to connect to
const char* ssid = "OLSN-AP";
const char* password = "12340987qwerpoiu";
// Networking setup
WebServer server(80);
// !!! Configure these to match the network you'll connect to
IPAddress local_IP(192, 168, 100, 245);
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 255, 0);
// Serial Setup
HardwareSerial unoSerial(2);
// UART
const int unoRX = 16;
const int unoTX = 17;
String uartBuffer;
// Data
String distance = "-1";
String water = "0";
String previous_water = "0";
String motor = "0";
String latest_status = "N/A";
bool isonline = false;

// Ping
const unsigned long PING_INTERVAL = 2000;
const unsigned long PING_TIMEOUT = 500;
unsigned long lastPing = 0;
unsigned long pingSent = 0;
bool waitingForPing = false;

void processUNO(String data) {
  data.trim();
  Serial.print("UNO: ");
  Serial.println(data);
  // Status check
  if (data == "ONLINE") {
    Serial.println("UNO is online.");
    isonline = true;
    waitingForPing = false;
    return;
  }
  // Parses JSON
  if (data.startsWith("{")) {
    int p;
    p = data.indexOf("\"distance\":");
    if (p >= 0) {
      p += 11;
      distance = data.substring(p, data.indexOf(',', p));
    }
    p = data.indexOf("\"water\":");
    if (p >= 0) {
      p += 8;
      water = data.substring(p, data.indexOf(',', p));

      float currentWater = water.toFloat();
      float previousWater = previous_water.toFloat();

      if (currentWater >= previousWater) {
        if (currentWater < 5.0)
          latest_status = "Normal condition";
        else if (currentWater <= 9.9)
          latest_status = "Level 1";
        else if (currentWater <= 14.9)
          latest_status = "Level 2";
        else
          latest_status = "Level 3";
      }
      else {
        if (previousWater > 10.0 && currentWater <= 10.0)
          latest_status = "Level decrease";
        else if (previousWater > 5.0 && currentWater <= 5.0)
          latest_status = "Level decrease";
        else if (currentWater < 5.0)
          latest_status = "Normal condition";
      }

      previous_water = water;
    }
    p = data.indexOf("\"motor\":");
    if (p >= 0) {
      p += 8;
      motor = data.substring(p, data.indexOf('}', p));
    }
  }
}

void uartIO() {
  while (unoSerial.available()) {
    char c = unoSerial.read();

    if (c == '\n') {
      processUNO(uartBuffer);
      uartBuffer = "";
    }
    else if (c != '\r') {
      uartBuffer += c;
    }
  }
}

void sendUNO(String command) {
  unoSerial.println(command);
}

void checkUNO() {
  if (!waitingForPing && millis() - lastPing >= PING_INTERVAL) {
    sendUNO("CHECK");
    pingSent = millis();
    lastPing = millis();
    waitingForPing = true;
    isonline = false;
  }

  if (waitingForPing && millis() - pingSent >= PING_TIMEOUT) {
    isonline = false;
    waitingForPing = false;
    Serial.println("UNO is offline.");
  }
}

void handleData() {
  String json = "{";
  json += "\"distance\":" + distance + ",";
  json += "\"water\":" + water + ",";
  json += "\"isonline\":" + String(isonline ? 1 : 0) + ",";
  json += "\"motor\":" + motor + ",";
  json += "\"status\":\"" + latest_status + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

// useless
void handleMotor() {
  if (!server.hasArg("state")) {
    server.send(400, "text/plain", "Missing state");
    return;
  }

  String state = server.arg("state");

  if (state == "1") {
    sendUNO("MOTOR_ON");
    server.send(200, "text/plain", "Motor ON");
  }
  else if (state == "0") {
    sendUNO("MOTOR_OFF");
    server.send(200, "text/plain", "Motor OFF");
  }
  else {
    server.send(400, "text/plain", "Invalid state");
  }
}

void setup() {
  // Serial debugging setup
  Serial.begin(115200);
  // UART for uno setup
  unoSerial.begin(9600, SERIAL_8N1, unoRX, unoTX);
  // Internet setup
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED)
    delay(100);

  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  server.on("/state", handleData);
  //server.on("/motor", handleMotor);

  server.begin();
}

void loop() {
  uartIO();
  checkUNO();
  server.handleClient();
}