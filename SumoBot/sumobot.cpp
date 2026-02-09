// https://github.com/adafruit/Adafruit_VL53L0X/blob/master/examples/vl53l0x_dual/vl53l0x_dual.ino
// Sehan M., Elya K., Vraj P.
// TERM - Mr. Wong
// 2026-02-XX
// Sumo Bot

// Imports
#include <bits/stdc++.h>
#include "Adafruit_VL53L0X.h"

// Motors
#define R_MOTOR_PWM 3 // PWMA
#define R_MOTOR_P1 4  // AIN1
#define R_MOTOR_P2 5  // AIN2
#define L_MOTOR_PWM 6 // PWMB
#define L_MOTOR_P1 7  // BIN1
#define L_MOTOR_P2 8  // BIN2
#define MOTOR_STBY 9  // STBY

// Ground Sensors
#define LDR_F A0
#define LDR_B A1

// Distance Sensors (VL53L0X Dual Sensor Setup)
#define LOX1_ADDRESS 0x30
#define LOX2_ADDRESS 0x31
#define SHT_LOX1 10
#define SHT_LOX2 11

// VL53L0X sensor objects
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();

// Measurement data structures
VL53L0X_RangingMeasurementData_t measure1;
VL53L0X_RangingMeasurementData_t measure2;

#define BASE_SPEED 150 // 1 - 255
#define DETECT_THRESHOLD 300
#define LDR_CONSTRAINT 600

void setID() {
  digitalWrite(SHT_LOX1, LOW);
  digitalWrite(SHT_LOX2, LOW);
  delay(10);
  digitalWrite(SHT_LOX1, HIGH);
  digitalWrite(SHT_LOX2, HIGH);
  delay(10);
  // activating LOX1 and resetting LOX2
  digitalWrite(SHT_LOX1, HIGH);
  digitalWrite(SHT_LOX2, LOW);

  // initing LOX1
  if (!lox1.begin(LOX1_ADDRESS)) {
    Serial.println(F("Failed to boot first VL53L0X"));
    while (1);
  }
  delay(10);
  // activating LOX2
  digitalWrite(SHT_LOX2, HIGH);
  delay(10);
  // initing LOX2
  if (!lox2.begin(LOX2_ADDRESS)) {
    Serial.println(F("Failed to boot second VL53L0X"));
    while (1);
  }
}

void read_dual_sensors() {
  lox1.rangingTest(&measure1, false); // pass in 'true' to get debug data printout!
  lox2.rangingTest(&measure2, false); // pass in 'true' to get debug data printout!
  // Sensor 1 
  Serial.print(F("1: "));
  if (measure1.RangeStatus != 4) { // if not out of range
    Serial.print(measure1.RangeMilliMeter);
  } else {
    Serial.print(F("Out of range"));
  }
  Serial.print(F(" "));
  // Sensor 2
  Serial.print(F("2: "));
  if (measure2.RangeStatus != 4) {
    Serial.print(measure2.RangeMilliMeter);
  } else {
    Serial.print(F("Out of range"));
  }
  Serial.println();
}

void setup() {
  pinMode(R_MOTOR_P1, OUTPUT);
  pinMode(R_MOTOR_P2, OUTPUT);
  pinMode(L_MOTOR_P1, OUTPUT);
  pinMode(L_MOTOR_P2, OUTPUT);

  pinMode(R_MOTOR_PWM, OUTPUT);
  pinMode(L_MOTOR_PWM, OUTPUT);
  pinMode(MOTOR_STBY, OUTPUT);

  digitalWrite(MOTOR_STBY, HIGH);

  Serial.begin(115200);
  while (!Serial) {
    delay(1);
  }

  pinMode(SHT_LOX1, OUTPUT);
  pinMode(SHT_LOX2, OUTPUT);
  Serial.println(F("Shutdown pins initialized..."));
  digitalWrite(SHT_LOX1, LOW);
  digitalWrite(SHT_LOX2, LOW);
  Serial.println(F("Both sensors in reset mode...(pins are low)"));
  Serial.println(F("Starting dual VL53L0X sensor initialization..."));
  setID();
  Serial.println(F("VL53L0X sensors initialized successfully!"));
}

void loop() {
  // Read dual VL53L0X sensors
  read_dual_sensors();
  int distL = (measure1.RangeStatus != 4) ? measure1.RangeMilliMeter : 5000;
  int distR = (measure2.RangeStatus != 4) ? measure2.RangeMilliMeter : 5000;
  
  int ldrF = analogRead(LDR_F);
  int ldrB = analogRead(LDR_B);
  
  bool frontWhite = ldrF > LDR_CONSTRAINT;
  bool backWhite = ldrB > LDR_CONSTRAINT;

  if (frontWhite && backWhite) { // pretty much lost senario
    motors(BASE_SPEED, -BASE_SPEED);
  } else if (frontWhite) {
    escapeB();
  } else if (backWhite) {
    escapeF();
  } else {
    if (distL < DETECT_THRESHOLD || distR < DETECT_THRESHOLD) {
      stopMotors();
    } else {
      spin();
    }
  }
  delay(100);
}

void motors(int right_speed, int left_speed) {
  if (right_speed > 0) {
    digitalWrite(R_MOTOR_P1, HIGH);
    digitalWrite(R_MOTOR_P2, LOW);
    analogWrite(R_MOTOR_PWM, right_speed);
  } else if (right_speed < 0) {
    digitalWrite(R_MOTOR_P1, LOW);
    digitalWrite(R_MOTOR_P2, HIGH);
    analogWrite(R_MOTOR_PWM, -right_speed);
  } else {
    analogWrite(R_MOTOR_PWM, 0);
  }
  if (left_speed > 0) {
    digitalWrite(L_MOTOR_P1, HIGH);
    digitalWrite(L_MOTOR_P2, LOW);
    analogWrite(L_MOTOR_PWM, left_speed);
  } else if (left_speed < 0) {
    digitalWrite(L_MOTOR_P1, LOW);
    digitalWrite(L_MOTOR_P2, HIGH);
    analogWrite(L_MOTOR_PWM, -left_speed);
  } else {
    analogWrite(L_MOTOR_PWM, 0);
  }
}

void stopMotors() {
  motors(0, 0);
}

void spin() {
  motors(BASE_SPEED, -BASE_SPEED);
}

void escapeF() {
  motors(BASE_SPEED, BASE_SPEED);
}

void escapeB() {
  motors(-BASE_SPEED, -BASE_SPEED);
}

int motorSpeed(double percentage) {
  return (int)(percentage*(255));
}
