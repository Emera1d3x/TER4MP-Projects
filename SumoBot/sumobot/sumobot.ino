// https://github.com/adafruit/Adafruit_VL53L0X/blob/master/examples/vl53l0x_dual/vl53l0x_dual.ino
// Sehan M., Elya K., Vraj P.
// TERM - Mr. Wong
// 2026-02-XX
// Sumo Bot

// Imports
#include <Wire.h>
#include <string.h> // For debug
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
#define LDR_F A0
#define LDR_B A1

// Distance Sensors (VL53L0X Dual Sensor & Ultrasonic Sensor)
  // Dist Sensor Objects
Adafruit_VL53L0X distSensorR = Adafruit_VL53L0X();
Adafruit_VL53L0X distSensorL = Adafruit_VL53L0X();
  // SHT pins
#define DISTR_SHT 2
#define DISTL_SHT 3
  // Memory Allocation
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

// Setup Dist Sensors
void setDistSensors(){
  setVL52L0X();
  setUltraSonic();
}

// Setup (DIST) VL52L0X Sensors
  // Weird method required when setting multiple VL52L0X
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

// Setup (DIST) UltraSonic Sensor
  // Trig Pin sends beam
  // Echo Pin detects beam
void setUltraSonic(){
  pinMode(ULTRA_TRIG, OUTPUT);
  pinMode(ULTRA_ECHO, INPUT);
}

// Setup (LDR) QRE
  // Actually not necessary
void setQRE(){
  pinMode(LDR_F, INPUT);
  pinMode(LDR_B, INPUT);
}

// Setup Motors
  // Regular and PWM for custom speed
void setMotors(){
  pinMode(R_MOTOR_P1, OUTPUT);
  pinMode(R_MOTOR_P2, OUTPUT);
  pinMode(L_MOTOR_P1, OUTPUT);
  pinMode(L_MOTOR_P2, OUTPUT);
  pinMode(R_MOTOR_PWM, OUTPUT);
  pinMode(L_MOTOR_PWM, OUTPUT);
}

// Setup
  // Called once as initialization
void setup() {
  Serial.begin(9600);
  setDistSensors();
  setMotors();
  setQRE();
  Serial.println("Setup Finished");
  delay(1000);
}

// Loop
  // Constantly called around 16 MHz but slowed due delays used
void loop() {
  // Read VL53L0X sensors for current information
  VL53L0X_RangingMeasurementData_t measureDISTR;
  VL53L0X_RangingMeasurementData_t measureDISTL;
  distSensorR.rangingTest(&measureDISTR, false);
  distSensorL.rangingTest(&measureDISTL, false);
  // Get distances for all sensors
  double distR = measurementCentimeters(measureDISTR.RangeMilliMeter);
  double distL = measurementCentimeters(measureDISTL.RangeMilliMeter);
  double distM = measureUltraSonic();
  // Debug for Dist
  char debugStringDist[100];
  snprintf(debugStringDist, sizeof(debugStringDist), "L: %.2f | M: %.2f | R: %.2f ", distL, distM, distR);
  Serial.print(debugStringDist);

  // Read QRE sensors 
  double whiteValF = analogRead(LDR_F);
  double whiteValB = analogRead(LDR_B);
  // Translate to detecting tape or not.
  bool whiteF = (LDR_DETECT_THRESHOLD > whiteValF);
  bool whiteB = (LDR_DETECT_THRESHOLD > whiteValB);
  // Debug for LDR
  /*char debugStringLDR[100];
  snprintf(debugStringLDR, sizeof(debugStringLDR), "F: %.2f | B: %.2f ", whiteValF, whiteValB);
  Serial.print(debugStringLDR);*/

  // Control
  if (whiteF || whiteB) { // urgent escape
    escape(whiteF, whiteB); 
  } else if (distR > DIST_DETECT_THRESHOLD && distL > DIST_DETECT_THRESHOLD && distM > DIST_DETECT_THRESHOLD_MIDDLE){ // no clue where opponent is
    search(); Serial.println("  SEARCHING");
  } else if (distR+5 < distL && distM < 70){ // opponent to left
    motors(motorSpeed(0), motorSpeed(1)); Serial.println("  LEFT");
  } else if (distR > distL+5 && distM < 70){ // opponent to right
    motors(motorSpeed(1), motorSpeed(0)); Serial.println("  RIGHT");
  } else { // opponent in front
    motors(motorSpeed(0.5), motorSpeed(0.5)); Serial.println("  FWD");
  }
  delay(10);
}

// Measure Ultrasonic sensor (Middle)
  // Send out wave and if and when it comes back
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

// Move Motors
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

// Stop Motors
void stopMotors() {
  motors(0, 0);
}

// Search for opponent bot
  // Search in a circle
void search() {
  motors(motorSpeed(SPIN_SPEED), -motorSpeed(SPIN_SPEED));
}

// Urgent on tape
  // Move away from the tape
void escape(bool front, bool back) {
  (front) ? motors(-1, -1) : motors(1, 1) ;
}

// Translates percentage to PWM val
int motorSpeed(double percentage) {
  return (int)(percentage*(255));
}

// Convert MM to CM
double measurementCentimeters(int millimeter){
  return (millimeter)/10;
}