#include <Arduino.h>
#include <Wire.h>

// I2C addresses
#define LCD_ADDRESS 0x3E
#define RGB_ADDRESS 0x62

// LCD Commands
#define LCD_CMD 0x80
#define LCD_DATA 0x40

void sendCommand(byte address, byte command) {
  Wire.beginTransmission(address);
  Wire.write(LCD_CMD);
  Wire.write(command);
  Wire.endTransmission();
}

void sendData(byte address, byte data) {
  Wire.beginTransmission(address);
  Wire.write(LCD_DATA);
  Wire.write(data);
  Wire.endTransmission();
}

// Initialize LCD
void lcdInit() {
  sendCommand(LCD_ADDRESS, 0x38); // Function set: 2-line mode
  delay(5);
  sendCommand(LCD_ADDRESS, 0x39); // Function set: Extended mode
  delay(5);
  sendCommand(LCD_ADDRESS, 0x14); // Internal OSC frequency
  sendCommand(LCD_ADDRESS, 0x70); // Contrast
  sendCommand(LCD_ADDRESS, 0x56); // Power/Contrast control
  sendCommand(LCD_ADDRESS, 0x6C); // Follower control
  delay(200);
  sendCommand(LCD_ADDRESS, 0x38); // Function set: Normal mode
  sendCommand(LCD_ADDRESS, 0x0C); // Display on, cursor off, blink off
  sendCommand(LCD_ADDRESS, 0x01); // Clear display
  delay(5);
}

// Set RGB backlight color
void setRGB(byte r, byte g, byte b) {
  Wire.beginTransmission(RGB_ADDRESS);
  Wire.write(0x00); // MODE1 register
  Wire.write(0x00); // Default
  Wire.endTransmission();

  Wire.beginTransmission(RGB_ADDRESS);
  Wire.write(0x01); // MODE2 register
  Wire.write(0x00); // Default
  Wire.endTransmission();

  Wire.beginTransmission(RGB_ADDRESS);
  Wire.write(0x08); // LEDOUT register
  Wire.write(0xAA); // All LEDs on
  Wire.endTransmission();

  Wire.beginTransmission(RGB_ADDRESS);
  Wire.write(0x04); // Red PWM register
  Wire.write(r);
  Wire.endTransmission();

  Wire.beginTransmission(RGB_ADDRESS);
  Wire.write(0x03); // Green PWM register
  Wire.write(g);
  Wire.endTransmission();

  Wire.beginTransmission(RGB_ADDRESS);
  Wire.write(0x02); // Blue PWM register
  Wire.write(b);
  Wire.endTransmission();
}

// Print text on the LCD
void lcdPrint(const char* message) {
  sendCommand(LCD_ADDRESS, 0x01); // Clear display
  delay(2);
  sendCommand(LCD_ADDRESS, 0x80); // Set DDRAM address to 0x00
  while (*message) {
    sendData(LCD_ADDRESS, *message++);
  }
}

void setup() {
  Wire.begin(2, 1); // SDA = GPIO2, SCL = GPIO1 for ESP32-C3

  // Initialize the LCD and RGB
  lcdInit();
  setRGB(0, 0, 255); // Set backlight color to blue

  // Display "Hello World!"
  lcdPrint("GG Gamers!!");
}

void loop() {
  // No additional actions in the loop
}