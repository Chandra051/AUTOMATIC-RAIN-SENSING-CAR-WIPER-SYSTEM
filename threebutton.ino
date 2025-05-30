#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

// OLED Display config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Servo setup
Servo servo1;
Servo servo2;

// WiFi Credentials
const char* ssid = "TOOFAN";         // Replace with your WiFi SSID
const char* password = "40022004"; // Replace with your WiFi password

WiFiServer server(80);

// Flags
String currentMode = "AUTO";
int sensorValue = 0;
int pos = 0;

void setup() {
  Serial.begin(115200);

  // Attach servos to GPIOs
  servo1.attach(D5); // GPIO14
  servo2.attach(D6); // GPIO12

  // OLED init
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  server.begin();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connected to WiFi");
  display.setCursor(0, 10);
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();

  Serial.println(WiFi.localIP());
}

void wipe(int delayTime) {
  for (pos = 180; pos >= 0; pos--) {
    servo1.write(pos);
    servo2.write(pos);
    delay(3);
  }
  for (pos = 0; pos <= 180; pos++) {
    servo1.write(pos);
    servo2.write(pos);
    delay(3);
  }
  delay(delayTime);
}

void displayStatus(const char* mode, const char* status) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Mode: ");
  display.println(mode);
  display.setCursor(0, 10);
  display.print("Rain: ");
  display.println(sensorValue);
  display.setCursor(0, 20);
  display.print("Status: ");
  display.println(status);
  display.display();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    if (request.indexOf("/manual") != -1) {
      currentMode = "MANUAL";
    } else if (request.indexOf("/automatic") != -1) {
      currentMode = "AUTO";
    } else if (request.indexOf("/stop") != -1) {
      currentMode = "STOP";
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("OK");
  }

  if (currentMode == "AUTO") {
    sensorValue = analogRead(A0);

    if (sensorValue > 900) {
      displayStatus("AUTO", "NIL");
      delay(1000);
    } else if (sensorValue > 700) {
      displayStatus("AUTO", "LOW");
      wipe(5000);
    } else if (sensorValue > 600) {
      displayStatus("AUTO", "MEDIUM");
      wipe(3000);
    } else if (sensorValue > 500) {
      displayStatus("AUTO", "HIGH");
      wipe(500);
    } else {
      displayStatus("AUTO", "HEAVY");
      wipe(300);
    }

  } else if (currentMode == "MANUAL") {
    displayStatus("MANUAL", "WIPING");
    wipe(1000);
  } else if (currentMode == "STOP") {
    displayStatus("STOP", "IDLE");
    delay(1000);
  }
}
