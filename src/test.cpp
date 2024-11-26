#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>

// I2C addresses
#define LCD_ADDRESS 0x3E
#define RGB_ADDRESS 0x62

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID"; // Replace with Eduroam SSID or whatever we used for pi's (Martin u up)
const char* password = "YOUR_WIFI_PASSWORD"; // same thing just password

// Server URL
const char* serverUrl = "https://constraints-guyana-occasionally-brook.trycloudflare.comt"; // Replace with server URL if this not correct.

// LCD Commands
#define LCD_CMD 0x80
#define LCD_DATA 0x40

// Function testing
void sendCommand(byte address, byte command);
void sendData(byte address, byte data);
void lcdInit();
void setRGB(byte r, byte g, byte b);
void lcdPrint(const char* message);
String fetchTextFromServer();

// Main code
void setup() {
  Wire.begin(2, 1); // SDA = Pin 2, SCL = Pin 1 for ESP32-C3

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize the LCD and RGB
  lcdInit();
  setRGB(0, 0, 255); // Set backlight color to blue

  // Fetch text from server and display on LCD
  String message = fetchTextFromServer();
  lcdPrint(message.c_str());
}

void loop() {
  // Amount of time between refreshes in milliseconds | Currently set to 30 seconds
  delay(30000); // Wait 30 seconds
  String message = fetchTextFromServer();
  lcdPrint(message.c_str());
}

// Function definitions for the main code above

void sendCommand(byte address, byte command) {
  Wire.beginTransmission(address); // Start I2C transmission to the specified address
  Wire.write(LCD_CMD); // Write LCD command in byte format not data
  Wire.write(command); // Send actual command byte to ESP32
  Wire.endTransmission(); // End I2C transmission
}

void sendData(byte address, byte data) {
  Wire.beginTransmission(address); // Starts I2C transmission to the specified address
  Wire.write(LCD_DATA); // Write LCD data in byte format not command
  Wire.write(data); // Send actual data byte to ESP32
  Wire.endTransmission(); // End I2C transmission
}

void lcdInit() {
  sendCommand(LCD_ADDRESS, 0x38); // Function set: 2-line mode
  delay(5);
  sendCommand(LCD_ADDRESS, 0x39); // Function set: Extended mode Adds more config options
  delay(5);
  sendCommand(LCD_ADDRESS, 0x14); // Internal OSC frequency (Clock Signal ICE ICE BABY)
  sendCommand(LCD_ADDRESS, 0x70); // Constast to Adjust the brightness of the display characters
  sendCommand(LCD_ADDRESS, 0x56); // Power/Contrast control More contrast control
  sendCommand(LCD_ADDRESS, 0x6C); // Follower control Enables interal Voltage booser for more stability in the display with delay
  delay(200);
  sendCommand(LCD_ADDRESS, 0x38); // Function set: Normal mode Now we leaf the extended mode
  sendCommand(LCD_ADDRESS, 0x0C); // Display on, cursor off, blink off | Essentially turning on the display with no writing cursor or blinking cursor
  sendCommand(LCD_ADDRESS, 0x01); // Clear display | Reset Display for next message
  delay(5);
}

void setRGB(byte r, byte g, byte b) {
  // Step 1: Set MODE1 register to 0x00 (default mode)
  Wire.beginTransmission(RGB_ADDRESS); // Begin I2C communication with the RGB controller
  Wire.write(0x00);                    // Select the MODE1 register (address 0x00)
  Wire.write(0x00);                    // Set MODE1 register value to 0x00 (normal mode)
  Wire.endTransmission();              // End the I2C communication

  // Step 2: Set MODE2 register to 0x00 (default mode)
  Wire.beginTransmission(RGB_ADDRESS); // Begin I2C communication again
  Wire.write(0x01);                    // Select the MODE2 register (address 0x01)
  Wire.write(0x00);                    // Set MODE2 register value to 0x00 (normal mode)
  Wire.endTransmission();              // End the I2C communication

  // Step 3: Configure LEDOUT register to enable all LEDs
  Wire.beginTransmission(RGB_ADDRESS); // Begin I2C communication
  Wire.write(0x08);                    // Select the LEDOUT register (address 0x08)
  Wire.write(0xAA);                    // Set LEDOUT register to 0xAA (turn on all LEDs)
  Wire.endTransmission();              // End the I2C communication

  // Step 4: Set PWM value for the red LED
  Wire.beginTransmission(RGB_ADDRESS); // Begin I2C communication
  Wire.write(0x04);                    // Select the red PWM register (address 0x04)
  Wire.write(r);                       // Set the intensity of the red LED (0-255)
  Wire.endTransmission();              // End the I2C communication

  // Step 5: Set PWM value for the green LED
  Wire.beginTransmission(RGB_ADDRESS); // Begin I2C communication
  Wire.write(0x03);                    // Select the green PWM register (address 0x03)
  Wire.write(g);                       // Set the intensity of the green LED (0-255)
  Wire.endTransmission();              // End the I2C communication

  // Step 6: Set PWM value for the blue LED
  Wire.beginTransmission(RGB_ADDRESS); // Begin I2C communication
  Wire.write(0x02);                    // Select the blue PWM register (address 0x02)
  Wire.write(b);                       // Set the intensity of the blue LED (0-255)
  Wire.endTransmission();              // End the I2C communication
}

void lcdPrint(const char* message) {
  sendCommand(LCD_ADDRESS, 0x01); // Clear display
  delay(2); // Delay for 2ms to wait for it to execute
  sendCommand(LCD_ADDRESS, 0x80); // Set DDRAM address to 0x00 | Set cursor to the beginning of the first line

    // Print each character in the message from the server
  while (*message) {
    sendData(LCD_ADDRESS, *message++);
  }
}

String fetchTextFromServer() {
    // check if the WiFi is connected
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http; // Declare an object of class HTTPClient
    http.begin(serverUrl); // Specify server URL via earlier defined serverUrl
    int httpResponseCode = http.GET(); // Initiate HTTP GET request and store the HTTP response code

    // Check if the HTTP response code is greater than 0 if so it is successful
    if (httpResponseCode > 0) {
      String payload = http.getString(); // Get response payload or string from the server
      http.end(); // End the connection
      return payload; // Return the fetched string
    } else {
      http.end(); // End the connection
      return "Error: HTTP GET Failed";
    }
  } else {
    return "Error: WiFi Not Connected";
  }
}
