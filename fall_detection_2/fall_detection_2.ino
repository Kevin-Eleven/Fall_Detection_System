#define BLYNK_TEMPLATE_ID "TMPL3kzed1MYj"
#define BLYNK_TEMPLATE_NAME "Fall Detection"
#define BLYNK_PRINT Serial

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP_Mail_Client.h> // 📌 SMTP Library for Email Sending

// WiFi credentials
char ssid[] = "Doctor\"s House"; // put your hotspot name here
char pass[] = "password";        // put your password here

// Blynk authentication token
char auth[] = "w_jIRRzlgar4K7yGjx-DrA0oAbyBRGTb";

// 📌 SMTP Email Credentials
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

// ✅ Replace with your credentials
#define SENDER_EMAIL "your email here"                     // 🔹 Use Gmail Address
#define SENDER_PASSWORD "put ur google app password here " // 🔹 Use Google App Password
// steps to create google app password are given in readme
#define RECIPIENT_EMAIL "reciever email here" // 🔹 Recipient Email Address

// Button Pin
#define BUTTON_PIN 0 // GPIO0 (D3)

// SMTP Session
SMTPSession smtp;

// MPU6050 I2C Address
const int MPU_addr = 0x68;
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ, Tmp;
float ax, ay, az, gx, gy, gz;
boolean fall = false, trigger1 = false, trigger2 = false, trigger3 = false;
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

    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");

    // Blynk.begin(auth, ssid, pass);
}

void loop()
{
    // Blynk.run();
    mpu_read();
    process_fall_detection();
    if (digitalRead(BUTTON_PIN) == LOW)
    { // 🔘 Button Pressed (LOW)
        Serial.println("📩 Button Pressed! Sending Email...");
        sendEmail();
        delay(5000); // 🕒 Avoid multiple presses
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

    ax = (AcX - (-155)) / 16384.00;
    ay = (AcY - (-165)) / 16384.00;
    az = (AcZ - (-661)) / 16384.00;

    gx = (GyX - (-491)) / 131.07;
    gy = (GyY - 525) / 131.07;
    gz = (GyZ - (-571)) / 131.07;

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

    // Convert acceleration to m/s^2
    Blynk.virtualWrite(V0, ax * 9.81);
    Blynk.virtualWrite(V1, ay * 9.81);
    Blynk.virtualWrite(V2, az * 9.81);
    Blynk.virtualWrite(V3, GyX);
    Blynk.virtualWrite(V4, GyY);
    Blynk.virtualWrite(V5, GyZ);
}

void process_fall_detection()
{
    float Raw_Amp = sqrt(ax * ax + ay * ay + az * az);
    int Amp = Raw_Amp * 10;

    Serial.print("Amp value = ");
    Serial.println(Amp);

    if (Amp <= 2 && trigger2 == false)
    { // If acceleration drops below 0.4g (free fall detected)
        trigger1 = true;
        Serial.println("🛑 TRIGGER 1 ACTIVATED - FREE FALL DETECTED!");
        maxImpact = 0; // Reset max impact when free fall starts
        delay(50);
    }

    if (trigger1 == true)
    {
        trigger1count++;
        maxImpact = max(maxImpact, Amp); // Keep track of the highest acceleration

        Serial.print("📈 Max Impact Recorded = ");
        Serial.println(maxImpact); // Debugging impact detection

        if (maxImpact >= 8)
        { // If impact is detected after free fall
            trigger2 = true;
            Serial.println("💥 TRIGGER 2 ACTIVATED - IMPACT DETECTED!");
            trigger1 = false;
            trigger1count = 0;
        }
    }

    if (trigger2 == true)
    {
        trigger2count++;
        angleChange = sqrt(gx * gx + gy * gy + gz * gz);
        Serial.print("📐 Angle Change = ");
        Serial.println(angleChange);

        if (angleChange >= 30 && angleChange <= 400)
        { // If orientation changes significantly
            trigger3 = true;
            trigger2 = false;
            trigger2count = 0;
            Serial.println("🔄 TRIGGER 3 ACTIVATED - ORIENTATION CHANGE DETECTED!");
        }
    }

    if (trigger3 == true)
    {
        trigger3count++;
        if (trigger3count >= 10)
        {
            angleChange = sqrt(gx * gx + gy * gy + gz * gz);
            Serial.print("🔍 Final Angle Change = ");
            Serial.println(angleChange);

            if (angleChange >= 0 && angleChange <= 10)
            { // If user remains in the fallen position
                fall = true;
                trigger3 = false;
                trigger3count = 0;
                Serial.println("⚠️ FALL DETECTED! Sending Alert...");
                sendEmail();
            }
            else
            { // User regained normal orientation
                trigger3 = false;
                trigger3count = 0;
                Serial.println("✅ TRIGGER 3 DEACTIVATED - NO FALL DETECTED.");
            }
        }
    }

    if (fall == true)
    { // If a fall was detected
        Serial.println("🚨 FALL CONFIRMED! Sending Email...");
        sendEmail();
        fall = false;
    }

    if (trigger2count >= 6)
    { // Allow time for orientation change
        trigger2 = false;
        trigger2count = 0;
        Serial.println("⏳ TRIGGER 2 DEACTIVATED - TIMEOUT");
    }

    if (trigger1count >= 6)
    { // Allow time for impact detection
        trigger1 = false;
        trigger1count = 0;
        Serial.println("⏳ TRIGGER 1 DEACTIVATED - TIMEOUT");
    }

    delay(100);
}
// 📌 Function to Send Email
void sendEmail()
{
    smtp.debug(1);

    ESP_Mail_Session session;
    session.server.host_name = SMTP_HOST;
    session.server.port = SMTP_PORT;
    session.login.email = SENDER_EMAIL;
    session.login.password = SENDER_PASSWORD;
    session.login.user_domain = "";

    SMTP_Message message;
    message.sender.name = "ESP8266 Button Test";
    message.sender.email = SENDER_EMAIL;
    message.subject = "📩 Button Pressed!";
    message.addRecipient("User", RECIPIENT_EMAIL);

    String htmlMsg = "<div style=\"color:#0000FF;\"><h1>ESP8266 Email Test</h1>"
                     "<p>This email was sent when the button was pressed.</p></div>";

    message.html.content = htmlMsg.c_str();
    message.text.charSet = "us-ascii";
    message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

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
        Serial.println("✅ Email Sent Successfully!");
    }
}
