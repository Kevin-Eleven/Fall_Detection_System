# Fall Detection System

This project uses a NodeMCU ESP8266 and MPU6050 accelerometer to detect falls and send email alerts when a fall is detected. The system can also trigger an alert manually using a push button.

## Table of Contents

- [Hardware Requirements](#hardware-requirements)
- [Software Setup](#software-setup)
  - [1. Install Arduino IDE](#1-install-arduino-ide)
  - [2. Install Required Libraries](#2-install-required-libraries)
  - [3. Setting up NodeMCU Board](#3-setting-up-nodemcu-board)
  - [4. Setting up Gmail App Password](#4-setting-up-gmail-app-password)
- [Configuring the Project](#configuring-the-project)
- [Uploading the Code](#uploading-the-code)
- [How it Works](#how-it-works)

## Hardware Requirements

- NodeMCU ESP8266 board
- MPU6050 Accelerometer/Gyroscope
- Push button
- Jumper wires
- USB cable for programming

## Software Setup

### 1. Install Arduino IDE

1. Download and install the Arduino IDE from the official website: [https://www.arduino.cc/en/software](https://www.arduino.cc/en/software)
2. Install the latest version suitable for your operating system.
3. Launch Arduino IDE after installation.

### 2. Install Required Libraries

#### Installing Libraries via Arduino Library Manager

1. In Arduino IDE, go to **Sketch > Include Library > Manage Libraries...**
2. Search for and install the following libraries:
   - **ESP8266WiFi** - For WiFi connectivity with NodeMCU
   - **Blynk** - For IoT connectivity (if using Blynk)
   - **ESP_Mail_Client** - For sending email alerts
   - **Wire** - For I2C communication (usually pre-installed)
   - **Time** - For time management

#### Installing Libraries from ZIP Files

If you have downloaded library ZIP files:

1. Go to **Sketch > Include Library > Add .ZIP Library...**
2. Navigate to the downloaded ZIP file and select it
3. Click "Open" to install

For the included libraries:

- **i2cdevlib-master.zip**: Install using the Add .ZIP Library method
- **MPU6050**: This library provides support for the accelerometer/gyroscope sensor
- **Blynk**: The Blynk IoT platform library for connecting to the Blynk service
- **BlynkESP8266_Lib**: Additional Blynk support for ESP8266
- **ESP_Mail_Client**: Library for sending emails using ESP8266/ESP32
- **Time**: Library for time-related functions

### 3. Setting up NodeMCU Board

#### Installing ESP8266 Board Support

1. Open Arduino IDE
2. Go to **File > Preferences**
3. In "Additional Boards Manager URLs" field, add: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
4. Click "OK"
5. Go to **Tools > Board > Boards Manager...**
6. Search for "esp8266"
7. Install "ESP8266 by ESP8266 Community"

#### Installing Required Drivers

If your NodeMCU doesn't appear in the Device Manager:

1. Press **Windows + X** and select **Device Manager**
2. Scroll down to the **Ports (COM & LPT)** section
3. Check if there's a device listed as "Silicon Labs CP210x USB to UART Bridge" or similar

If the device is not appearing or shows with a warning icon:

1. Download the Silicon Labs CP210x driver from: [https://www.silabs.com/documents/public/software/CP210x_Windows_Drivers_with_Serial_Enumeration.zip](https://www.silabs.com/documents/public/software/CP210x_Windows_Drivers_with_Serial_Enumeration.zip)
2. Extract the ZIP file to a folder
3. Run **CP210xVCPInstaller_x64.exe** (for 64-bit systems) or **CP210xVCPInstaller_x86.exe** (for 32-bit systems)
4. Follow the installation prompts
5. Reconnect your NodeMCU and check Device Manager again

### 4. Setting up Gmail App Password

Since we're using SMTP to send email alerts, you need to set up an App Password for your Gmail account:

1. Go to your Google Account at [https://myaccount.google.com/](https://myaccount.google.com/)
2. Select **Security** from the left navigation panel
3. Under "Signing in to Google," select **2-Step Verification** and turn it on if not already enabled
   - Follow the steps to set up 2-Step Verification
4. After enabling 2-Step Verification, go back to the Security page
5. Under "Signing in to Google," select **App passwords**
   - You might need to sign in again
6. Click **Select app** and choose "Mail"
7. Click **Select device** and choose "Other (Custom name)"
8. Enter a name like "ESP8266 Fall Detection"
9. Click **Generate**
10. Google will display a 16-character app password (without spaces)
11. Copy this password and keep it secure - you'll need it for the project

## Configuring the Project

1. Open the `fall_detection_2.ino` file in Arduino IDE
2. Update the following variables with your information:

   ```cpp
   // WiFi credentials
   char ssid[] = "Your_WiFi_SSID";
   char pass[] = "Your_WiFi_Password";

   // Blynk authentication token (if using Blynk)
   char auth[] = "Your_Blynk_Auth_Token";

   // SMTP Email Credentials
   #define SENDER_EMAIL "your-email@gmail.com"
   #define SENDER_PASSWORD "your-16-char-app-password" // The app password you generated
   #define RECIPIENT_EMAIL "recipient-email@example.com"
   ```

## Uploading the Code

1. Connect your NodeMCU to your computer using a USB cable
2. In Arduino IDE, select the correct board:
   - Go to **Tools > Board > ESP8266 Boards > NodeMCU 1.0 (ESP-12E Module)**
3. Select the correct port:
   - Go to **Tools > Port** and select the COM port where your NodeMCU is connected
4. Click the **Upload** button (right arrow icon) to compile and upload the code to your NodeMCU

## How it Works

The system uses an MPU6050 accelerometer/gyroscope to detect falls by monitoring:

1. Free fall detection - when acceleration drops significantly
2. Impact detection - when a sudden high acceleration occurs
3. Orientation change - when the person's orientation changes significantly

When a fall is detected or when the button is pressed, the system sends an email alert to the designated recipient using the SMTP server.

The system also allows integration with Blynk IoT platform for monitoring the accelerometer values in real-time through a mobile app.
