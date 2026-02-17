// https://github.com/adafruit/Adafruit_VL53L0X/blob/master/examples/vl53l0x_dual/vl53l0x_dual.ino
// Sehan M., Elya K., Vraj P.
// TERM - Mr. Wong
// 2026-02-XX
// Sumo Bot

// Imports
#include <Wire.h>
#include "lib/Adafruit_VL53L0X/src/Adafruit_VL53L0X.h"

// Motors
  // Right Motor
#define R_MOTOR_PWM 5 // PWMA
#define R_MOTOR_P1 7 // AIN1
#define R_MOTOR_P2 8 // AIN2
  // Left Motor
#define L_MOTOR_PWM 6 // PWMB
#define L_MOTOR_P1 9 // BIN1
#define L_MOTOR_P2 10 // BIN2

// Ground Sensors (QRE1113 IR Sensor)
#define LDR_A A0
#define LDR_B A1
#define LDR_C A2

// Distance Sensors (VL53L0X Dual Sensor Setup)
  //
Adafruit_VL53L0X distR = Adafruit_VL53L0X();
Adafruit_VL53L0X distL = Adafruit_VL53L0X();
  // 
#define DISTR_SHT 2
#define DISTL_SHT 3
  //
#define DISTR_ADDRESS 0x30
#define DISTL_ADDRESS 0x31
  // Distance Sensor (The other one)

// Additional Constants
#define SPIN_SPEED 0.9 // Spin Speed Percentage
#define DIST_DETECT_THRESHOLD 850 // 
#define LDR_DETECT_THRESHOLD 250 // 


// Weird method required for setting multiple VL52L0X
  // Start with both off, restart one sensor at one memory address, and restart the other at another memoory address via XSHUT pin
void setVL52L0X() {
  pinMode(DISTR_SHT, OUTPUT);
  pinMode(DISTL_SHT, OUTPUT);
  // Off
  digitalWrite(DISTR_SHT, LOW);
  digitalWrite(DISTL_SHT, LOW);
  delay(10);
  // Restart R
  digitalWrite(DISTR_SHT, HIGH);
  delay(10);
  distR.begin(DISTR_ADDRESS);
  // Restart L
  digitalWrite(DISTL_SHT, HIGH);
  delay(10);
  distL.begin(DISTL_ADDRESS);
  Serial.println("VL52L0X Set");
}


void setQRE(){

}

void setupMotors(){
  pinMode(R_MOTOR_P1, OUTPUT);
  pinMode(R_MOTOR_P2, OUTPUT);
  pinMode(L_MOTOR_P1, OUTPUT);
  pinMode(L_MOTOR_P2, OUTPUT);
  pinMode(R_MOTOR_PWM, OUTPUT);
  pinMode(L_MOTOR_PWM, OUTPUT);
}

void setup() {
  Serial.begin(9600);
  setVL52L0X();
  setMotors();
  setQRE();
  Serial.println("Setup Finished");
  delay(1000);
}

void loop() {
  // Read dual VL53L0X sensors
  VL53L0X_RangingMeasurementData_t measureDISTR;
  VL53L0X_RangingMeasurementData_t measureDISTL;

  distR.rangingTest(&measureDISTR, false);
  distL.rangingTest(&measureDISTL, false);
  
  int distR = measurementCentimeters(measureDISTR.RangeMilliMeter);
  int distL = measurementCentimeters(measureDISTL.RangeMilliMeter);
  
  string debugDist = "L: " + to_string(distL) = " | R: " + to_string(distR);
  Serial.println(debugDist);

  bool whiteA = (LDR_DETECT_THRESHOLD > analogRead(LDR_A));
  bool whiteB = (LDR_DETECT_THRESHOLD > analogRead(LDR_B));
  bool whiteC = (LDR_DETECT_THRESHOLD > analogRead(LDR_C));

  // Escape Conditions (Urgent)
    // idk where the placements are yet
  // Non-Escape Conditions
  if (distR > DIST_DETECT_THRESHOLD || distL > DIST_DETECT_THRESHOLD){
    // where tf is ts bot
    search();
  } else if (distR < distL+2){
    motors(motorSpeed(0.2), motorSpeed(0.1));
  } else if (distR+2 > distL){
    motors(motorSpeed(0.1), motorSpeed(0.2));
  } else if (abs(distL - distR) > 2) {
    motors(motorSpeed(0.3), motorSpeed(0.3));
  }
  delay(10);
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

void search() {
  motors(motorSpeed(SPIN_SPEED), -motorSpeed(SPIN_SPEED));
}

int motorSpeed(double percentage) {
  return (int)(percentage*(255));
}

double measurementCentimeters(int millimeter){
  return (millimeter)/10;
}