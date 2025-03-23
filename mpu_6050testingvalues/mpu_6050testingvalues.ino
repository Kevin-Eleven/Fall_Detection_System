#include <Wire.h>

const uint8_t MPU_addr = 0x68;  // Change to 0x69 if needed
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

void setup() {
    Wire.begin(D2, D1);  // SDA = D2, SCL = D1
    Serial.begin(115200);
    
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B);
    Wire.write(0);  
    Wire.endTransmission(true);  // Wake up MPU6050
    
    Serial.println("MPU6050 Initialized");
}

void loop() {
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14, true);

    AcX = Wire.read() << 8 | Wire.read();
    AcY = Wire.read() << 8 | Wire.read();
    AcZ = Wire.read() << 8 | Wire.read();
    Tmp = Wire.read() << 8 | Wire.read();
    GyX = Wire.read() << 8 | Wire.read();
    GyY = Wire.read() << 8 | Wire.read();
    GyZ = Wire.read() << 8 | Wire.read();

    Serial.print("AcX = "); Serial.print(AcX);
    Serial.print(" | AcY = "); Serial.print(AcY);
    Serial.print(" | AcZ = "); Serial.println(AcZ);

    Serial.print("GyX = "); Serial.print(GyX);
    Serial.print(" | GyY = "); Serial.print(GyY);
    Serial.print(" | GyZ = "); Serial.println(GyZ);

    Serial.println("--------------------------------");
    delay(1000);
}
