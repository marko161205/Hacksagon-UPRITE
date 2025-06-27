//DO NOT TAMPER WITH THIS CODE !!!!!!!!!!!!!

//Using kalman Library by TKJ Electronics and nRF51822 chip with Arduino IDE using Sandeep Mistry's BLE Peripheral Library and 


#include <Wire.h>
#include <BLEPeripheral.h>
#include <math.h>
#include "Kalman.h"

#define MPU1_ADDR 0x68
#define MPU2_ADDR 0x69

//
int Buzzpin = 17;                                                                             //Vibrator
int BatPin = 20;                                                                              //Battery
float BatVal;

float batMax = 3.3;
float batMin = 2.05;

BLEPeripheral blePeripheral;                                                                   //Joshi UUID
BLEService imuService("12345678-1234-1234-1234-1234567890ab");
BLECharacteristic imuChar("abcd1234-5678-90ab-cdef-1234567890ab", BLERead | BLENotify, 20);

Kalman kalmanRoll1, kalmanPitch1;
Kalman kalmanRoll2, kalmanPitch2;

unsigned long timer = 0;


void setup() {
  Serial.begin(9600);
  
  pinMode(Buzzpin,OUTPUT);
  pinMode(BatPin,INPUT);
  
  Wire.setPins(0, 30);
  Wire.begin();

  initMPU(MPU1_ADDR);
  initMPU(MPU2_ADDR);

  blePeripheral.setLocalName("Uprite");
  blePeripheral.setAdvertisedServiceUuid(imuService.uuid());
  blePeripheral.addAttribute(imuService);
  blePeripheral.addAttribute(imuChar);
  blePeripheral.begin();

  delay(1000);

  // Initial reading for Kalman filter
  int16_t ax, ay, az, gx, gy, gz;
  readMPU(MPU1_ADDR, ax, ay, az, gx, gy, gz);
  float roll = atan2(ay, az) * 180.0 / PI;
  float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;
  kalmanRoll1.setAngle(roll);
  kalmanPitch1.setAngle(pitch);

  readMPU(MPU2_ADDR, ax, ay, az, gx, gy, gz);
  roll = atan2(ay, az) * 180.0 / PI;
  pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;
  kalmanRoll2.setAngle(roll);
  kalmanPitch2.setAngle(pitch);

  timer = millis();
}

void loop() {
  blePeripheral.poll();

  BatVal = ((analogRead(BatPin) - batMin)/(batMax - batMin))*100.00;
  
  if (imuChar.written()) {
    String received = String((const char*)imuChar.value());
    if (received == "0"){
      for(int _ = 0; _ < 3; _++){                                     //Buzzer
        digitalWrite(Buzzpin,HIGH);
        delay(500);
        digitalWrite(Buzzpin,LOW);
        delay(500);
      }
    }
  }

  int16_t ax1, ay1, az1, gx1, gy1, gz1;
  int16_t ax2, ay2, az2, gx2, gy2, gz2;

  readMPU(MPU1_ADDR, ax1, ay1, az1, gx1, gy1, gz1);
  readMPU(MPU2_ADDR, ax2, ay2, az2, gx2, gy2, gz2);

  float dt = (float)(millis() - timer) / 1000.0;
  timer = millis();

  // Convert gyro to deg/s
  float gyroX1 = gx1 / 131.0, gyroY1 = gy1 / 131.0;
  float roll1Acc = atan2(ay1, az1) * 180.0 / PI;
  float pitch1Acc = atan2(-ax1, sqrt(ay1 * ay1 + az1 * az1)) * 180.0 / PI;
  float roll1 = kalmanRoll1.getAngle(roll1Acc, gyroX1, dt);
  float pitch1 = kalmanPitch1.getAngle(pitch1Acc, gyroY1, dt);
  float yaw1 = gz1 / 131.0;

  float gyroX2 = gx2 / 131.0, gyroY2 = gy2 / 131.0;
  float roll2Acc = atan2(ay2, az2) * 180.0 / PI;
  float pitch2Acc = atan2(-ax2, sqrt(ay2 * ay2 + az2 * az2)) * 180.0 / PI;
  float roll2 = kalmanRoll2.getAngle(roll2Acc, gyroX2, dt);
  float pitch2 = kalmanPitch2.getAngle(pitch2Acc, gyroY2, dt);
  float yaw2 = gz2 / 131.0;

  float dRoll = roll2 - roll1;
  float dPitch = pitch2 - pitch1;
  float dYaw = yaw2 - yaw1;

  char buffer[20];
  snprintf(buffer, sizeof(buffer), "%.1f,%.1f,%.1f,%.1f", dRoll, dPitch, dYaw, BatVal);
  imuChar.setValue((const unsigned char*)buffer, strlen(buffer));

  delay(10);  // smaller delay for smoother filtering
}

void initMPU(uint8_t addr) {
  Wire.beginTransmission(addr);
  Wire.write(0x6B); // PWR_MGMT_1
  Wire.write((uint8_t)0);
  Wire.endTransmission();
}

void readMPU(uint8_t addr, int16_t &ax, int16_t &ay, int16_t &az,
             int16_t &gx, int16_t &gy, int16_t &gz) {
  Wire.beginTransmission(addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(addr, 14, true);

  if (Wire.available() < 14) {
    ax = ay = az = gx = gy = gz = 0;
    return;
  }

  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();
  Wire.read(); Wire.read();  // skip temp
  gx = Wire.read() << 8 | Wire.read();
  gy = Wire.read() << 8 | Wire.read();
  gz = Wire.read() << 8 | Wire.read();
}
