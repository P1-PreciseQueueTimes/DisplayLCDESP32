#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>

/**
 * @brief ESP32 LCD Display with RGB Backlight.
 * @note 
 * This program is connecting our ESP32 to a Wifi network, it then initializes a LCD with RGB Backlight,
 * goes to our server and fetches text to display on the LCD every 30 seconds.
 * 
 * @details about the Program and how it works;
 * The program follows these steps when running:
 * 1. It initializes serial communication for debugging purposes.
 * 2. It initializes the I2C communication with the LCD and RGB backlight.
 * 3. It tries to Connect to AAU-1-DAY wifi network with all possible passwords seen since last update to code.
 * 4. If it successfully connects to the WiFi, it initializes the LCD and RGB backlight.
 * 5. Again if it successfully connects to the WiFi, it fetches text from the server and displays it on the LCD.
 * 6. It then proceeds to fetch or ask for a new text string every 30 seconds, if a new one is provided it updates every 30 seconds.
 * 
 * @note This note Shows the logic/functions of the program:
 * The program includes the following functions/logic:
 * - setup(): Initializes the system and performs the initial setup.
 * - loop(): Continuously fetches and updates the text on the LCD.
 * - connectToWiFi(): Connects to the WiFi network using multiple passwords.
 * - sendCommand(): Sends a command to the LCD via I2C.
 * - sendData(): Sends data to the LCD via I2C.
 * - lcdInit(): Initializes the LCD.
 * - setRGB(): Sets the RGB backlight color.
 * - lcdPrint(): Displays a message on the LCD.
 * - fetchTextFromServer(): Fetches text from the server. 
 */


// WiFi credentials for AAU-1-DAY
const char* ssid = "AAU-1-DAY"; // Guest WiFi SSID
// Guest WiFi passwords in array format
const char* password[] = {
    "vain93force", // Today's password
    "loss73side", // Tomorrow's password
    "hope77send", // Day after tomorrow's password
    "dads15gray" // 4th day's password
}; 

// I2C addresses for LCD and RGB backlight
#define LCD_ADDRESS 0x3E
#define RGB_ADDRESS 0x62

// Server URL
const char* serverUrl = "https://completing-understood-cannon-senegal.trycloudflare.com/get/testing/esp32";

// Function prototypes/declarations tells compiler about the function allowing the function to be called before it's defined.
void connectToWiFi();
void sendCommand(byte address, byte command   );
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
  setRGB(255, 255, 128); // Set backlight color to lukewarm yellow

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

/**
 * @brief Connect to the WiFi network using multiple passwords. (WPA2-Personal)
 */
void connectToWiFi() {
    WiFi.disconnect(true); // Disconnect any previous connections
    WiFi.mode(WIFI_STA);   // Set ESP32 to Station mode
    bool connected = false; // Flag to indicate successful connection

    for (int i = 0; i < sizeof(password) / sizeof(password[0]); i++) {
        Serial.printf("Attempting to connect with password %d: %s\n", i + 1, password[i]);
        WiFi.begin(ssid, password[i]); // Try connecting with the current password
        int retries = 0; // Track connection attempts
        while (WiFi.status() != WL_CONNECTED && retries < 20) { // Retry for a maximum of 20 seconds
            delay(5000);
            Serial.print(".");
            retries++;
        }
        if (WiFi.status() == WL_CONNECTED) { // Check if connected
            Serial.printf("\nSuccessfully connected to WiFi with password %d: %s\n", i + 1, password[i]);
            connected = true;
            break; // Exit loop after successful connection
        } else {
            Serial.printf("\nFailed to connect with password %d: %s\n", i + 1, password[i]);
        }
        WiFi.disconnect(true); // Disconnect and try the next password
    }

    if (!connected) {
        Serial.println("\nFailed to connect to WiFi with any provided password.");
    }
}


/**
 * @brief Send a command to the LCD via I2C.
 * @param address I2C address of the device.
 * @param command Command byte to send.
 */
void sendCommand(byte address, byte command) {
  Wire.beginTransmission(address);
  Wire.write(0x80); // Command mode
  Wire.write(command);
  Wire.endTransmission();
}

/**
 * @brief Send data to the LCD via I2C.
 * @param address I2C address of the device.
 * @param data Data byte to send.
 */
void sendData(byte address, byte data) {
  Wire.beginTransmission(address);
  Wire.write(0x40); // Data mode
  Wire.write(data);
  Wire.endTransmission();
}

/**
 * @brief Initialize the LCD.
 */
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

/**
 * @brief Set the RGB backlight color.
 * @param r Red intensity (0-255).
 * @param g Green intensity (0-255).
 * @param b Blue intensity (0-255).
 */
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

/**
 * @brief Display a message on the LCD.
 * @param message Null-terminated string to display.
 */
void lcdPrint(const char* message) {
  sendCommand(LCD_ADDRESS, 0x01); // Clear display
  delay(2);
  sendCommand(LCD_ADDRESS, 0x80); // Set cursor to the beginning of the first line
  while (*message) {
    sendData(LCD_ADDRESS, *message++);
  }
}

/**
 * @brief Fetch text from the server.
 * @return String containing the server response or an error message.
 */
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