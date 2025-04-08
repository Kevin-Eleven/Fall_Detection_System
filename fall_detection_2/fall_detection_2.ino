#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP_Mail_Client.h> // 📌 SMTP Library for Email Sending
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>

// WiFi credentials
char ssid[] = "Unknown";
char pass[] = "nopassword";

// 📌 SMTP Email Credentials
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

// ✅ Replace with your credentials
#define SENDER_EMAIL "harisahmad200411@gmail.com"
#define SENDER_PASSWORD "" // 🔹 Use Google App Password
#define RECIPIENT_EMAIL "unnamed114477@gmail.com"

// Button Pin
#define BUTTON_PIN 0 // GPIO0 (D3)

// GPS Module Pins (Changed from D2/D1 to D6/D5)
#define GPS_RX 12 // D6 (GPIO12)
#define GPS_TX 14 // D5 (GPIO14)
SoftwareSerial gpsSerial(GPS_RX, GPS_TX);
TinyGPSPlus gps;

// SMTP Session
SMTPSession smtp;

// MPU6050 I2C Address
const int MPU_addr = 0x68;
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ, Tmp;
float ax, ay, az, gx, gy, gz;
boolean fall = false, trigger1 = false, trigger2 = false, trigger3 = false, impactDetected = false;
byte trigger1count = 0, trigger2count = 0, trigger3count = 0;
int angleChange = 0;
int maxImpact = 0;

void setup()
{
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP); // ✅ Internal pull-up resistor
    Wire.begin();
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);

    gpsSerial.begin(9600);

    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");

}

void loop()
{
 
    mpu_read();
    process_fall_detection();
    while (gpsSerial.available())
    {
        gps.encode(gpsSerial.read());
    }

    if (digitalRead(BUTTON_PIN) == LOW)
    {
        Serial.println("📩 Button Pressed! Sending Emergency Email...");
        sendEmergencyEmail("🚨 BUTTON PRESSED! I NEED HELP!", "I have pressed the emergency button. Please check on me immediately.");
        delay(5000);
    }
    delay(100);
}

void mpu_read()
{
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);
    Wire.endTransmission(false);

    // ✅ Fix applied here
    Wire.requestFrom((uint8_t)MPU_addr, (size_t)14, (bool)true);

    AcX = Wire.read() << 8 | Wire.read();
    AcY = Wire.read() << 8 | Wire.read();
    AcZ = Wire.read() << 8 | Wire.read();
    Tmp = Wire.read() << 8 | Wire.read();
    GyX = Wire.read() << 8 | Wire.read();
    GyY = Wire.read() << 8 | Wire.read();
    GyZ = Wire.read() << 8 | Wire.read();

    //  AcX Offset: 17332
    // AcY Offset: 478
    // AcZ Offset: -15546
    // GyX Offset: -697
    // GyY Offset: 193
    // GyZ Offset: -319

    ax = (AcX - (17332)) / 16384.00;
    ay = (AcY - (478)) / 16384.00;
    az = (AcZ - (-15546)) / 16384.00;

    gx = (GyX - (-697)) / 131.07;
    gy = (GyY - 193) / 131.07;
    gz = (GyZ - (-319)) / 131.07;

    // 📌 Print acceleration values to Serial Monitor
    Serial.print("Acceleration (m/s²) - X: ");
    Serial.print(ax * 9.81);
    Serial.print(" | Y: ");
    Serial.print(ay * 9.81);
    Serial.print(" | Z: ");
    Serial.println(az * 9.81);

    // 📌 Print gyroscope values to Serial Monitor
    Serial.print("Gyroscope (°/s) - X: ");
    Serial.print(gx);
    Serial.print(" | Y: ");
    Serial.print(gy);
    Serial.print(" | Z: ");
    Serial.println(gz);
    Serial.println("----------------------------");

    // in units of g 
}

void process_fall_detection()
{
    float Raw_Amp = sqrt(ax * ax + ay * ay + az * az);
    int Amp = Raw_Amp * 10;

    Serial.print("Amp value = ");
    Serial.println(Amp);

    if (Amp <= 4 && !trigger2)
    {
        trigger1 = true;
        Serial.println("🛑 FREE FALL DETECTED!");
        maxImpact = 0;
        delay(50);
    }

    if (trigger1)
    {
        trigger1count++;
        maxImpact = max(maxImpact, Amp);

        if (maxImpact >= 6)
        {
            trigger2 = true;
            impactDetected = true;
            Serial.println("💥 IMPACT DETECTED! Sending Impact Alert...");
            sendEmergencyEmail("💥 IMPACT DETECTED!", "A strong impact has been detected. Please check on me immediately!");
            trigger1 = false;
            trigger1count = 0;
        }
    }

    if (trigger2)
    {
        trigger2count++;
        angleChange = sqrt(gx * gx + gy * gy + gz * gz);

        if (angleChange >= 30 && angleChange <= 400)
        {
            trigger3 = true;
            trigger2 = false;
            trigger2count = 0;
            Serial.println("🔄 ORIENTATION CHANGE DETECTED!");
        }
    }

    if (trigger3)
    {
        trigger3count++;
        if (trigger3count >= 10)
        {
            angleChange = sqrt(gx * gx + gy * gy + gz * gz);
            if (angleChange <= 20)
            {
                fall = true;
                trigger3 = false;
                trigger3count = 0;
                Serial.println("⚠️ FALL DETECTED! Sending Emergency Alert...");
                sendEmergencyEmail("🚨 FALL DETECTED!", "A fall has been detected. I might need immediate assistance!");
            }
            else
            {
                trigger3 = false;
                trigger3count = 0;
                Serial.println("✅ No Fall Detected.");
            }
        }
    }
}

// 📩 Function to Send Emergency Email
void sendEmergencyEmail(String subject, String body)
{
    smtp.debug(1);
    ESP_Mail_Session session;
    session.server.host_name = SMTP_HOST;
    session.server.port = SMTP_PORT;
    session.login.email = SENDER_EMAIL;
    session.login.password = SENDER_PASSWORD;

    SMTP_Message message;
    message.sender.name = "Fall Detection System";
    message.sender.email = SENDER_EMAIL;
    message.subject = subject;
    message.addRecipient("Emergency Contact", RECIPIENT_EMAIL);

    String locationInfo = getLocation();
    body += "<br><br><b>" + locationInfo + "</b>";

    String htmlMsg = "<div style=\"color:#FF0000;\"><h1>" + subject + "</h1><p>" + body + "</p></div>";
    message.html.content = htmlMsg.c_str();

    if (!smtp.connect(&session))
    {
        Serial.println("❌ SMTP Connection Failed!");
        return;
    }

    if (!MailClient.sendMail(&smtp, &message))
    {
        Serial.println("❌ Email Failed: " + smtp.errorReason());
    }
    else
    {
        Serial.println("✅ EMERGENCY EMAIL SENT SUCCESSFULLY!");
    }
}

String getLocation()
{
    if (gps.location.isValid())
    {
        float latitude = gps.location.lat();
        float longitude = gps.location.lng();
        String googleMapsLink = "https://www.google.com/maps/search/?api=1&query=" + String(latitude, 6) + "," + String(longitude, 6);
        return "📍 GPS Location: " + String(latitude, 6) + ", " + String(longitude, 6) + "<br><a href='" + googleMapsLink + "' target='_blank'>📍 View on Google Maps</a>";
    }
    Serial.println("⚠️ GPS data unavailable. Trying IP-based location...");
    return getIPGeolocation();
}

String getIPGeolocation()
{
    WiFiClient client;
    HTTPClient http;
    http.begin(client, "http://ip-api.com/json"); // ✅ Fixed API usage

    int httpCode = http.GET(); // Make GET request
    String payload = "";

    if (httpCode > 0)
    {
        payload = http.getString();
        Serial.println("📩 IP Geolocation Response: " + payload);

        // Parse JSON response
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error)
        {
            String city = doc["city"].as<String>();
            String country = doc["country"].as<String>();
            float lat = doc["lat"].as<float>();
            float lon = doc["lon"].as<float>();

            // ✅ Convert IP location to Google Maps link
            String googleMapsLink = "https://www.google.com/maps/search/?api=1&query=" +
                                    String(lat, 6) + "," + String(lon, 6);

            String locationInfo = "🌍 Location: " + city + ", " + country +
                                  " (" + String(lat, 6) + ", " + String(lon, 6) + ")<br>" +
                                  "<a href=\"" + googleMapsLink + "\" target=\"_blank\">📍 View on Google Maps</a>";

            return locationInfo;
        }
        else
        {
            Serial.println("❌ JSON Parsing Error");
        }
    }
    else
    {
        Serial.println("❌ HTTP Request Failed");
    }

    http.end();
    return "⚠️ Unable to fetch IP location";
}
