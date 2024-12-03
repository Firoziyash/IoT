#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "Home-Automation"; // Wi-Fi SSID
const char* password = "12345678";     // Wi-Fi Password

ESP8266WebServer server(80);

// Relay Pins
const int relayPins[] = {D1, D2, D3, D4};
const int motionPin = D5; // PIR Motion Sensor Pin

bool relayStates[4] = {false, false, false, false};
unsigned long lastMotionTime = 0;
const unsigned long motionTimeout = 10000; // 10 seconds

void setup() {
  // Setup serial communication
  Serial.begin(115200);
  delay(10);

  // Configure relay pins as OUTPUT
  for (int i = 0; i < 4; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH); // Initially turn off relays
  }

  // Configure motion sensor pin
  pinMode(motionPin, INPUT);

  // Start Wi-Fi in AP mode
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Created!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Define routes for the web server
  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);

  // Start the server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  // Motion detection logic
  if (digitalRead(motionPin) == HIGH) {
    lastMotionTime = millis();
  }

  // Turn off all appliances if no motion is detected for 10 seconds
  if (millis() - lastMotionTime >= motionTimeout) {
    turnOffAllRelays();
  }
}

// Handle the root webpage
void handleRoot() {
  String html = "<html><head><title>Home Automation</title></head><body>";
  html += "<h1>Control Appliances</h1>";
  for (int i = 0; i < 4; i++) {
    html += "<p>Relay " + String(i + 1) + ": ";
    html += relayStates[i] ? "ON" : "OFF";
    html += " <a href='/toggle?relay=" + String(i) + "'>Toggle</a></p>";
  }
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// Handle relay toggling
void handleToggle() {
  if (server.hasArg("relay")) {
    int relay = server.arg("relay").toInt();
    if (relay >= 0 && relay < 4) {
      relayStates[relay] = !relayStates[relay];
      digitalWrite(relayPins[relay], relayStates[relay] ? LOW : HIGH);
    }
  }
  handleRoot(); // Redirect to the main page
}

// Turn off all relays
void turnOffAllRelays() {
  for (int i = 0; i < 4; i++) {
    relayStates[i] = false;
    digitalWrite(relayPins[i], HIGH);
  }
}
