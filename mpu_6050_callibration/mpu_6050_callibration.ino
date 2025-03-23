#include <Wire.h>

const int MPU_addr = 0x68;  // I2C address of MPU6050

// Offsets (to be calculated)
long AcX_offset = 0, AcY_offset = 0, AcZ_offset = 0;
long GyX_offset = 0, GyY_offset = 0, GyZ_offset = 0;

void setup() {
    Serial.begin(115200);
    Wire.begin();

    // Wake up MPU6050
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(0);    // Wake it up
    Wire.endTransmission(true);

    Serial.println("🚀 Starting Calibration... Keep MPU6050 **flat and stable**!");

    calibrateMPU6050();
}

void calibrateMPU6050() {
    const int numSamples = 2000; // Increase for better accuracy

    long AcX_total = 0, AcY_total = 0, AcZ_total = 0;
    long GyX_total = 0, GyY_total = 0, GyZ_total = 0;

    for (int i = 0; i < numSamples; i++) {
        Wire.beginTransmission(MPU_addr);
        Wire.write(0x3B);  // Start reading from accelerometer registers
        Wire.endTransmission(false);
        Wire.requestFrom(MPU_addr, 14, true);

        int16_t AcX = Wire.read() << 8 | Wire.read();
        int16_t AcY = Wire.read() << 8 | Wire.read();
        int16_t AcZ = Wire.read() << 8 | Wire.read();
        int16_t Tmp = Wire.read() << 8 | Wire.read();
        int16_t GyX = Wire.read() << 8 | Wire.read();
        int16_t GyY = Wire.read() << 8 | Wire.read();
        int16_t GyZ = Wire.read() << 8 | Wire.read();

        AcX_total += AcX;
        AcY_total += AcY;
        AcZ_total += AcZ - 16384; // Subtract gravity (1g = 16384)
        GyX_total += GyX;
        GyY_total += GyY;
        GyZ_total += GyZ;

        delay(3); // Small delay to avoid I2C overload
    }

    // Calculate averages
    AcX_offset = AcX_total / numSamples;
    AcY_offset = AcY_total / numSamples;
    AcZ_offset = AcZ_total / numSamples;
    GyX_offset = GyX_total / numSamples;
    GyY_offset = GyY_total / numSamples;
    GyZ_offset = GyZ_total / numSamples;

    // Print the calculated offsets
    Serial.println("\n✅ Calibration Done! Use these offsets in your code:");
    Serial.print("    AcX Offset: "); Serial.println(AcX_offset);
    Serial.print("    AcY Offset: "); Serial.println(AcY_offset);
    Serial.print("    AcZ Offset: "); Serial.println(AcZ_offset);
    Serial.print("    GyX Offset: "); Serial.println(GyX_offset);
    Serial.print("    GyY Offset: "); Serial.println(GyY_offset);
    Serial.print("    GyZ Offset: "); Serial.println(GyZ_offset);
}

void loop() {
    // Calibration is done in setup(), so no need to loop.
}
