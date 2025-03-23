#include <ESP8266WiFi.h>
#include <ESP_Mail_Client.h>  // ✅ SMTP Library

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

// ✅ Replace with your credentials
#define SENDER_EMAIL "harisahmad200411@gmail.com"
#define SENDER_PASSWORD "acax rqhp ubrh xglv"  // 🔹 Use Google App Password
#define RECIPIENT_EMAIL "unnamed114477@gmail.com"

// WiFi credentials
const char* ssid = "Doctor\"s House";
const char* password = "password";

// Button Pin
#define BUTTON_PIN 0  // GPIO0 (D3)

// SMTP Session
SMTPSession smtp;

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);  // ✅ Internal pull-up resistor

    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");
}

void loop() {
    if (digitalRead(BUTTON_PIN) == LOW) {  // 🔘 Button Pressed (LOW)
        Serial.println("📩 Button Pressed! Sending Email...");
        sendEmail();
        delay(5000);  // 🕒 Avoid multiple presses
    }
}

// 📌 Function to Send Email
void sendEmail() {
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

    if (!smtp.connect(&session)) {
        Serial.println("❌ SMTP Connection Failed!");
        return;
    }

    if (!MailClient.sendMail(&smtp, &message)) {
        Serial.println("❌ Email Failed: " + smtp.errorReason());
    } else {
        Serial.println("✅ Email Sent Successfully!");
    }
}
