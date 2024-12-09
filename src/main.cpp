#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>

/**
 * @file main.cpp
 * @authors William, Josephine, Martin, Asbjørn, Muhsin
 * @date last changed 09/12/2024.
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
