#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>

/**
 * @file main.cpp
 * @authors William, Josephine, Martin, Asbjørn, Muhsin.
 * @date last changed 09/12/24.
 * 
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
    "wound54sound", // Today's password
    "loss73side", // Tomorrow's password
    "hope77send", // Day after tomorrow's password
    "quiet34coal" // 4th day's password
}; 

// I2C addresses for LCD and RGB backlight
#define LCD_ADDRESS 0x3E
#define RGB_ADDRESS 0x62

// Server URL
const char* serverUrl = "https://airports-shopping-squad-newton.trycloudflare.com/get/testing/esp32";

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
 *  
 * @details
 * We take a null-terminated string also known as a C-string as input and display it on the LCD.
 * It can take both Single and two-line messages. If a message contains the newline character '\n', then we know it is a two-line message.
 * This functions splits the text into two different lines, before displaying it (it renders the text before the newline statement on the first line and the text after the newline statement on the second line).
 * Each line has a limit of 16 characters due to the LCD's Line width. If the message does not contain a newline, the entire message is displayed on the first line (up to 16 characters).
 * 
 * @param message Null-terminated string (C-String) to display.
 */
void lcdPrint(const char* message) {
  sendCommand(LCD_ADDRESS, 0x01); // Clear display
  delay(2);
  // Pointers to manage the splitting of the message into two lines.
  const char* line1 = message; // Start of the first line                   
  const char* line2 = strchr(message, '\n'); // Look for a newline statement/charecter.
  
  if (line2) { // if a newline charecter is found, it will  split the  message into two lines.
    size_t line1Length = line2 - message; // This Calculates the length of the first line.
    char firstLine[17] = {0}; // This Makes a Buffer for the first line. (16 characters + null terminator)
    strncpy(firstLine, line1, line1Length > 16 ? 16 : line1Length); // Copies all Charecters (first line) up to 16  characters.
    
    line2++; // Move pointer past the newline to the start of the second line.
    char secondLine[17] = {0}; // This Makes a Buffer for  the second line. (16 characters + null terminator)
    strncpy(secondLine, line2, 16); // Copies all Charecters (second line) up to 16  characters.

    // Display First line on LCD
    sendCommand(LCD_ADDRESS, 0x80); // set  the cursor to the beginning of the first line.
    for (int i = 0; i < strlen(firstLine); i++) {
      sendData(LCD_ADDRESS, firstLine[i]); // Write each charecter to the LCD.
    }
    //  Display Second line on LCD
    sendCommand(LCD_ADDRESS, 0xC0); // set  the cursor to the beginning of the second line.
    for (int i = 0; i < strlen(secondLine); i++) {
      sendData(LCD_ADDRESS, secondLine[i]); // Write each charecter to the LCD.
    }
  } else {
    sendCommand(LCD_ADDRESS, 0x80); // if no newline charecter is found the entire message is displayed on the first line.
    for (int i = 0; i < strlen(message) && i < 16; i++) {
      sendData(LCD_ADDRESS, message[i]); // Write each charecter to the LCD. (up to 16 characters)
    }
  }
}


/**
 * @brief Fetch text from the server.
 * 
 * This function attempts to fetch text data from a server using an HTTP GET request.
 * It first checks if the device is connected to WiFi. If connected, it sends an HTTP GET request
 * to the server and retrieves the response payload. If the request fails or if the device is not
 * connected to WiFi, it returns an appropriate error message.
 * 
 * @return String containing the server response or an error message.
 * 
 * @note The `payload` variable contains the text data returned by the server in response to the HTTP GET request.
 * The `httpResponseCode` is an integer because it represents the HTTP status code, which is a numeric value.
 * The `payload` is a String because it represents the actual content of the server's response, which is text.
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