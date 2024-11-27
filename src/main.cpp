#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>

// WiFi credentials for AAU-1-DAY
const char* ssid = "AAU-1-DAY"; // Guest WiFi SSID
const char* password = "guru46watch"; // Guest WiFi password

// I2C addresses for LCD and RGB backlight
#define LCD_ADDRESS 0x3E
#define RGB_ADDRESS 0x62

// Server URL
const char* serverUrl = "https://happens-flights-foundations-allergy.trycloudflare.com/get/testing/esp32";

// Function prototypes
void connectToWiFi();
void sendCommand(byte address, byte command);
void sendData(byte address, byte data);
void lcdInit();
void setRGB(byte r, byte g, byte b);
void lcdPrint(const char* message);
String fetchTextFromServer();

void setup() {
  Serial.begin(115200); // Initialize serial communication for debugging
  Wire.begin(2, 1);     // SDA = Pin 2, SCL = Pin 1 for ESP32-C3

  // Connect to WiFi
  connectToWiFi();

  // Initialize the LCD and RGB
  lcdInit();
  setRGB(255, 255, 128); // Set backlight color to blue

  // Fetch text from the server and display it on the LCD
  String message = fetchTextFromServer();
  lcdPrint(message.c_str());
}

void loop() {
  // Fetch text from the server every 30 seconds
  delay(30000);
  String message = fetchTextFromServer();
  lcdPrint(message.c_str());
}

// Function to connect to WiFi with WPA2-Personal
void connectToWiFi() {
  WiFi.disconnect(true); // Disconnect any previous connections
  WiFi.mode(WIFI_STA);   // Set ESP32 to Station mode
  WiFi.begin(ssid, password); // Start WiFi connection with SSID and password

  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) { // Wait for connection
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
}

// Send a command to the LCD via I2C
void sendCommand(byte address, byte command) {
  Wire.beginTransmission(address);
  Wire.write(0x80); // Command mode
  Wire.write(command);
  Wire.endTransmission();
}

// Send data to the LCD via I2C
void sendData(byte address, byte data) {
  Wire.beginTransmission(address);
  Wire.write(0x40); // Data mode
  Wire.write(data);
  Wire.endTransmission();
}

// Initialize the LCD
void lcdInit() {
  sendCommand(LCD_ADDRESS, 0x38); // Function set: 2-line mode
  delay(5);
  sendCommand(LCD_ADDRESS, 0x39); // Function set: Extended mode
  delay(5);
  sendCommand(LCD_ADDRESS, 0x14); // Internal oscillator frequency
  sendCommand(LCD_ADDRESS, 0x70); // Contrast
  sendCommand(LCD_ADDRESS, 0x56); // Power/contrast control
  sendCommand(LCD_ADDRESS, 0x6C); // Follower control
  delay(200);
  sendCommand(LCD_ADDRESS, 0x38); // Function set: Normal mode
  sendCommand(LCD_ADDRESS, 0x0C); // Display on, cursor off
  sendCommand(LCD_ADDRESS, 0x01); // Clear display
  delay(5);
}

// Set RGB backlight color
void setRGB(byte r, byte g, byte b) {
  Wire.beginTransmission(RGB_ADDRESS);
  Wire.write(0x00); Wire.write(0x00); // Set MODE1 register
  Wire.endTransmission();
  Wire.beginTransmission(RGB_ADDRESS);
  Wire.write(0x01); Wire.write(0x00); // Set MODE2 register
  Wire.endTransmission();
  Wire.beginTransmission(RGB_ADDRESS);
  Wire.write(0x08); Wire.write(0xAA); // Enable all LEDs
  Wire.endTransmission();
  Wire.beginTransmission(RGB_ADDRESS);
  Wire.write(0x04); Wire.write(r); // Red intensity
  Wire.endTransmission();
  Wire.beginTransmission(RGB_ADDRESS);
  Wire.write(0x03); Wire.write(g); // Green intensity
  Wire.endTransmission();
  Wire.beginTransmission(RGB_ADDRESS);
  Wire.write(0x02); Wire.write(b); // Blue intensity
  Wire.endTransmission();
}

// Display a message on the LCD
void lcdPrint(const char* message) {
  sendCommand(LCD_ADDRESS, 0x01); // Clear display
  delay(2);
  sendCommand(LCD_ADDRESS, 0x80); // Set cursor to the beginning of the first line
  while (*message) {
    sendData(LCD_ADDRESS, *message++);
  }
}

// Fetch text from the server
String fetchTextFromServer() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String payload = http.getString(); // Get response payload
      http.end();
      return payload;
    } else {
      String ErrorMessage = "Error: HTTP GET Failed, Response/Error Code: " + String(httpResponseCode);
      http.end();
      return ErrorMessage;
    }
  } else {
    String WifiError = "Error: WiFi Disconnected/Not Connected";
    Serial.println(WifiError); // should give new improved debug info
    return WifiError;
  }
}