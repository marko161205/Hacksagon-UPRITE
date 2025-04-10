#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

#define SDA_PIN 21
#define SCL_PIN 22

MPU6050 mpu1(0x68);  
MPU6050 mpu2(0x69);

bool dmpReady1 = false;
uint8_t fifoBuffer1[64];

bool dmpReady2 = false;
uint8_t fifoBuffer2[64];

float yprdiff[3];

float ypr1[3];
float ypr2[3];

float bend = 10.0;

void setupMPU(MPU6050 &mpu, bool &dmpReady) {
  Serial.println(F("Initializing MPU6050..."));
  mpu.initialize();
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

  Serial.println(F("Initializing DMP..."));

  if (mpu.dmpInitialize() == 0) {
    mpu.CalibrateAccel(100);
    mpu.CalibrateGyro(100);
    //mpu.PrintActiveOffsets());
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);
    dmpReady = true;
    Serial.println(F("DMP ready!"));
  } else {
    Serial.print(F("DMP Initialization failed (code "));
    //Serial.print(mpu.dmpInitialize());
    Serial.println(F(")"));
  }
}

void newBend() {
  if (SerialBT.available()) {
    String input = SerialBT.readStringUntil('\n');
    input.trim();

    if (input.startsWith("SET_BEND: ")) {
      float newBend = input.substring(10).toFloat();
      if (newBend > 0.0 && newBend < 90.0) {
        bend = newBend;
      }
      else{
        SerialBT.println("Invalid bend value.");
      }
    }
  }
}

void readMpuDiff(MPU6050 &mpu1,MPU6050 &mpu2, uint8_t *fifoBuffer1, uint8_t *fifoBuffer2) {
  if (mpu1.getFIFOCount() < 42 || mpu2.getFIFOCount() < 42) return; //check if good data

  Quaternion q1;
  Quaternion q2;
  VectorFloat gravity;


  if (mpu1.dmpGetCurrentFIFOPacket(fifoBuffer1) && mpu2.dmpGetCurrentFIFOPacket(fifoBuffer2)) {
    mpu1.dmpGetQuaternion(&q1, fifoBuffer1);
    mpu1.dmpGetGravity(&gravity, &q1);
    mpu1.dmpGetYawPitchRoll(ypr1, &q1, &gravity);

    // Convert to degrees
    float yaw1 = ypr1[0] * 180.0 / M_PI;
    float pitch1 = ypr1[1] * 180.0 / M_PI;
    float roll1 = ypr1[2] * 180.0 / M_PI;

    mpu2.dmpGetQuaternion(&q2, fifoBuffer2);
    mpu2.dmpGetYawPitchRoll(ypr2, &q2, &gravity);

    float yaw2 = ypr2[0] * 180.0 / M_PI;
    float pitch2 = ypr2[1] * 180.0 / M_PI;
    float roll2 = ypr2[2] * 180.0 / M_PI;

    yprdiff[0] = abs(yaw2-yaw1);
    yprdiff[1] = abs(pitch2-pitch1);
    yprdiff[2] = abs(roll2-roll1);

    Serial.println("");
  }
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("OnePlus Nord CE2 Lite 5G");  // Bluetooth name

  Wire.begin(SDA_PIN, SCL_PIN);   
  Wire.setClock(400000);          
  Serial.println("Waiting for MPU...");
  delay(1000);

  Serial.println("Setting up MPU1...");
  setupMPU(mpu1, dmpReady1);
  
  Serial.println("Setting up MPU2...");
  setupMPU(mpu2, dmpReady2);
  
}

void loop() {
  newBend();

  if (!dmpReady1 || !dmpReady2) return;
  readMpuDiff(mpu1, mpu2, fifoBuffer1, fifoBuffer2);
  if(yprdiff[0]>bend || yprdiff[1]>bend || yprdiff[2]>bend)
  Serial.println("BAD POSTURE");
  else
  Serial.println("GOOD POSTURE");
  delay(10);  

  SerialBT.printf("YPR_DIFF: %.2f, %.2f, %.2f \n", yprdiff[0], yprdiff[1], yprdiff[2]);
}
