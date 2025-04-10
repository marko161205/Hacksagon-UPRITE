#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

// Define custom I2C pins for ESP32
#define SDA_PIN 21
#define SCL_PIN 22

MPU6050 mpu1(0x68);  // Default I2C address for MPU6050

bool dmpReady1 = false;
uint8_t devStatus1;
uint8_t fifoBuffer1[64];

Quaternion q;
VectorFloat gravity;
float ypr[3];

void setupMPU(MPU6050 &mpu, bool &dmpReady) {
  Serial.println(F("Initializing MPU6050..."));
  mpu.initialize();
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

  Serial.println(F("Initializing DMP..."));
  devStatus1 = mpu.dmpInitialize();

  if (devStatus1 == 0) {
    mpu.CalibrateAccel(100);
    mpu.CalibrateGyro(100);
    mpu.PrintActiveOffsets();
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);
    dmpReady = true;
    Serial.println(F("DMP ready!"));
  } else {
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus1);
    Serial.println(F(")"));
  }
}

void readMpuData(MPU6050 &mpu, uint8_t *fifoBuffer) {
  if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) {
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

    // Convert to degrees for easy reading
    float yaw = ypr[0] * 180 / M_PI;
    float pitch = ypr[1] * 180 / M_PI;
    float roll = ypr[2] * 180 / M_PI;

    Serial.print("Yaw: ");
    Serial.print(yaw);
    Serial.print("\tPitch: ");
    Serial.print(pitch);
    Serial.print("\tRoll: ");
    Serial.println(roll);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);   
  Wire.setClock(400000);          
  delay(1000);
  setupMPU(mpu1, dmpReady1);
}

void loop() {
  if (!dmpReady1) return;
  readMpuData(mpu1, fifoBuffer1);
  delay(10);  
}