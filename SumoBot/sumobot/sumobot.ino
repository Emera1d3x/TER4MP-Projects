// https://github.com/adafruit/Adafruit_VL53L0X/blob/master/examples/vl53l0x_dual/vl53l0x_dual.ino
// Sehan M., Elya K., Vraj P.
// TERM - Mr. Wong
// 2026-02-XX
// Sumo Bot

// Imports
#include <Wire.h>
//#include "lib/Adafruit_VL53L0X/src/Adafruit_VL53L0X.h"
#include <Adafruit_VL53L0X.h>

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
#define LDR_R A0
#define LDR_L A1
#define LDR_B A2

// Distance Sensors (VL53L0X Dual Sensor Setup)
  //
Adafruit_VL53L0X distSensorR = Adafruit_VL53L0X();
Adafruit_VL53L0X distSensorL = Adafruit_VL53L0X();
  // 
#define DISTR_SHT 2
#define DISTL_SHT 3
  //
#define DISTR_ADDRESS 0x30
#define DISTL_ADDRESS 0x31
  // Distance Sensor (The other one)
#define ULTRA_TRIG 11
#define ULTRA_ECHO 12

// Additional Constants
#define SPIN_SPEED 1 // Spin Speed Percentage
#define DIST_DETECT_THRESHOLD 50 // 
#define DIST_DETECT_THRESHOLD_MIDDLE 50 // 
#define LDR_DETECT_THRESHOLD 250 // 

void setDistSensors(){
  setVL52L0X();
  setUltraSonic();
}
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
  distSensorR.begin(DISTR_ADDRESS);
  // Restart L
  digitalWrite(DISTL_SHT, HIGH);
  delay(10);
  distSensorL.begin(DISTL_ADDRESS);
  Serial.println("VL52L0X Set");
}

void setUltraSonic(){
  pinMode(ULTRA_TRIG, OUTPUT);
  pinMode(ULTRA_ECHO, INPUT);
}

void setQRE(){
  pinMode(LDR_R, INPUT);
  pinMode(LDR_L, INPUT);
  pinMode(LDR_B, INPUT);
}

void setMotors(){
  pinMode(R_MOTOR_P1, OUTPUT);
  pinMode(R_MOTOR_P2, OUTPUT);
  pinMode(L_MOTOR_P1, OUTPUT);
  pinMode(L_MOTOR_P2, OUTPUT);
  pinMode(R_MOTOR_PWM, OUTPUT);
  pinMode(L_MOTOR_PWM, OUTPUT);
}

void setup() {
  Serial.begin(9600);
  setDistSensors();
  setMotors();
  setQRE();
  Serial.println("Setup Finished");
  delay(1000);
}

void loop() {
  // Read dual VL53L0X sensors
  VL53L0X_RangingMeasurementData_t measureDISTR;
  VL53L0X_RangingMeasurementData_t measureDISTL;

  distSensorR.rangingTest(&measureDISTR, false);
  distSensorL.rangingTest(&measureDISTL, false);
  
  double distR = measurementCentimeters(measureDISTR.RangeMilliMeter);
  double distL = measurementCentimeters(measureDISTL.RangeMilliMeter);
  double distM = measureUltraSonic();
  
  Serial.print("L: ");
  Serial.print(distL);
  Serial.print(" | M: ");
  Serial.print(distM);
  Serial.print(" | R: ");
  Serial.print(distR);
  //string debugDist = "L: " + to_string(distL) = " | M: " + to_string(distM) + " | R: " + to_string(distR);
  //Serial.println(debugDist);

  double whiteR = analogRead(LDR_R);
  double whiteL = analogRead(LDR_L);
  double whiteB = analogRead(LDR_B);
  /*bool whiteR = (LDR_DETECT_THRESHOLD > analogRead(LDR_R));
  bool whiteL = (LDR_DETECT_THRESHOLD > analogRead(LDR_L));
  bool whiteB = (LDR_DETECT_THRESHOLD > analogRead(LDR_B));*/
  //Serial.print("L: ");
  //Serial.print(whiteL);
  //Serial.print(" | B: ");
  //Serial.print(whiteB);
  //Serial.print(" | R: ");
  //Serial.println(whiteR);
  //Serial.println();

  
  // Escape Conditions (Urgent)
    // idk where the placements are yet
  // Non-Escape Conditions
  if (distR > DIST_DETECT_THRESHOLD && distL > DIST_DETECT_THRESHOLD && distM > DIST_DETECT_THRESHOLD_MIDDLE){
    // where tf is ts bot
    search(); Serial.println("  SEARCHING");
  } else if (distR+5 < distL && distM < 70){
    motors(motorSpeed(0), motorSpeed(1)); Serial.println("  LEFT");
  } else if (distR > distL+5 && distM < 70){
    motors(motorSpeed(1), motorSpeed(0)); Serial.println("  RIGHT");
  } else {
    motors(motorSpeed(0.5), motorSpeed(0.5)); Serial.println("  FWD");
  }
  delay(10);
}

double measureUltraSonic(){
  digitalWrite(ULTRA_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRA_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRA_TRIG, LOW);
  double duration = pulseIn(ULTRA_ECHO, HIGH, 30000);
  double distance = duration*0.034/2;
  return distance;
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