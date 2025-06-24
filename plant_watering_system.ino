//importing libraries
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

//storing used pins in variable
#define relayPin    D5    // GPIO14
#define moisturePin A0    // analog input

const int dry         = 452;  // dry calibration
const int wet         = 234;  // wet calibration
const int errorMargin = 3;    // accomodate for sensor errors

// —————— LCD Setup ——————
LiquidCrystal_I2C lcd(0x27, 16, 2);

// —————— Wi-Fi credentials ——————
const char* ssid     = "";
const char* password = "";

// —————— HTTP Server ——————
ESP8266WebServer server(8080); //node on the network it is running on
const char* deviceName = "Plant Watering Device"; //device name to identify on web application

// —————— Plant thresholds ——————
//plant object to hold the properties
struct PlantConfig {
  int minVWC;
  int maxVWC;
  int optimalVWC;
  bool configured;
} plantConfig = {0, 0, 0, false}; //default state

// —————— State ——————
//state variables for watering conditions
bool   isWatering     = false;
bool   thresholdFlag  = false;
String statusMessage  = "";
bool   printedConfig  = false;  // print thresholds once

// — forward declarations —
void connectToWiFi();
void setupWebServer();
int  moisturePercentage(int raw);
void printMoistureAndStatus(int pct, const String &msg);

void setup() {
  Serial.begin(115200); //buad rate for serial communication

  // Relay off initally
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  //Configuring LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0); lcd.print("Plant Watering");
  lcd.setCursor(0,1); lcd.print("System Ready!");
  delay(1500);
  lcd.clear();

  connectToWiFi();
  setupWebServer();
  server.begin();

  //Print statements for debugging
  Serial.println("Server listening on port 8080");
  lcd.setCursor(0,0); lcd.print("Waiting for");
  lcd.setCursor(0,1); lcd.print("configuration");
}

void loop() {
  // Always listen for status/configure
  server.handleClient();

  if (!plantConfig.configured) return;

  // Print the thresholds once after config
  if (!printedConfig) {
    Serial.println(">>> Received plant config:");
    Serial.printf("minVWC=%d%%  maxVWC=%d%%  optimalVWC=%d%%\n\n",
                  plantConfig.minVWC,
                  plantConfig.maxVWC,
                  plantConfig.optimalVWC);
    printedConfig = true;
  }

  int raw = analogRead(moisturePin); //read value from sensor
  int pct = moisturePercentage(raw); //convert it to percentage
  Serial.printf("Moisture: %d%%\n", pct); 

  //---------Instructions for when to Water------------
  // Start watering
  if (pct < (plantConfig.minVWC + errorMargin)
      && !isWatering && !thresholdFlag) {
    digitalWrite(relayPin, HIGH);
    isWatering    = true;
    thresholdFlag = false;
    statusMessage = "Watering...     ";
  }
  // Stop watering
  if (isWatering && pct > (plantConfig.optimalVWC + errorMargin)) {
    digitalWrite(relayPin, LOW);
    isWatering    = false;
    thresholdFlag = true;
    statusMessage = "Moisture OK     ";
  }
  // Reset flag
  if (pct > (plantConfig.optimalVWC + 10)) thresholdFlag = false;

  printMoistureAndStatus(pct, statusMessage); //print message on LCD
  delay(2000);
}

void connectToWiFi() {
  //printing on the LCD
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("Connecting WiFi");
  lcd.setCursor(0,1); lcd.print(ssid);

  WiFi.begin(ssid, password); //beginning a connection on the network
  while (WiFi.status() != WL_CONNECTED) { //checking if it is connected
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  //Serial.print("IP = "); Serial.println(WiFi.localIP()); //retrieve the IP address for debugging purposes 

  lcd.clear();
  lcd.setCursor(0,0); lcd.print("WiFi Connected");
  //lcd.setCursor(0,1); lcd.print(WiFi.localIP().toString()); //LCD only accepts string so we have to convert
  delay(2000);
  lcd.clear();
}

void setupWebServer() {
  // Device discovery endpoint
  //checking if plant is configured and has the configured values
  server.on("/status", HTTP_GET, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json",
                String("{\"device\":\"") + deviceName +
                "\",\"configured\":" + (plantConfig.configured ? "true" : "false") +
                "}");
  });

  // Configure endpoint
  //checking if it recieved the data in JSON format
  server.on("/configure", HTTP_POST, []() {
    if (!server.hasArg("plain")) {
      Serial.println("ERROR: No JSON data received");
      server.send(400, "application/json", "{\"error\":\"No JSON\"}");
      return;
    }
    
    String jsonData = server.arg("plain");
    //debugging purposes to check length of data recieved and what the values are
    Serial.println("=== RECEIVED CONFIGURATION ===");
    Serial.println("Raw JSON: " + jsonData);
    Serial.println("JSON length: " + String(jsonData.length()));
    
    DynamicJsonDocument doc(256);
    auto err = deserializeJson(doc, jsonData);
    if (err) {
      Serial.println("JSON parsing error: " + String(err.c_str()));
      server.send(400, "application/json", "{\"error\":\"Bad JSON\"}");
      return;
    }
    
    Serial.println("JSON parsed successfully");
    
    // Check if fields exist and get their values
    Serial.println("Checking JSON fields:");
    Serial.println("Has minVWC: " + String(doc.containsKey("minVWC") ? "YES" : "NO"));
    Serial.println("Has maxVWC: " + String(doc.containsKey("maxVWC") ? "YES" : "NO"));
    Serial.println("Has optimalVWC: " + String(doc.containsKey("optimalVWC") ? "YES" : "NO"));
    
    // Taking the required values from the JSON and assigning it to our variable
    plantConfig.minVWC = doc["minVWC"];
    plantConfig.maxVWC = doc["maxVWC"];
    plantConfig.optimalVWC = doc["optimalVWC"];
    plantConfig.configured = true;

    //debugging + seeing assigned values
    Serial.println("=== PARSED VALUES ===");
    Serial.println("minVWC: " + String(plantConfig.minVWC));
    Serial.println("maxVWC: " + String(plantConfig.maxVWC));
    Serial.println("optimalVWC: " + String(plantConfig.optimalVWC));
    Serial.println("=====================");
    
    server.send(200, "application/json", "{\"success\":true}");
    Serial.println("→ Configuration received");
  });

  // CORS preflight (requesting the server for access)
  server.on("/configure", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(200);
  });
}

int moisturePercentage(int raw) {
  int p = (dry - raw) * 100 / (dry - wet);
  return constrain(p, 0, 100);
}

void printMoistureAndStatus(int pct, const String &msg) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Moisture: ");
  lcd.print(pct);
  lcd.print("%");
  lcd.setCursor(0,1);
  lcd.print(msg);
} 