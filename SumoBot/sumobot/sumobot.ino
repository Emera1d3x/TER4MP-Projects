// https://github.com/adafruit/Adafruit_VL53L0X/blob/master/examples/vl53l0x_dual/vl53l0x_dual.ino
// Sehan M., Elya K., Vraj P.
// TERM - Mr. Wong
// 2026-02-XX
// Sumo Bot

// Imports
#include <Wire.h>
#include <time.h>
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
#define DIST_DETECT_THRESHOLD 18 // 
#define DIST_DETECT_THRESHOLD_MIDDLE 15 // 
#define LDR_DETECT_THRESHOLD 110 // 

// Search 
  // Abandon spin search after time for a vertex search, prevents statemates 
bool spinMode = false;
bool vertexMode = false;
unsigned long spinStartTime = 0;

// Normalized Values
  // Perhaps smoothens dist values, reduces jitters, and mitigate errors 
double normalizedVals[3] = {0.0, 0.0, 0.0};

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
  pinMode(LDR_R, INPUT);
  pinMode(LDR_L, INPUT);
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
  delay(1);
  motors(motorSpeed(1), motorSpeed(1));
  delay(500);
}

// Loop
  // Constantly called around 16 MHz but slowed due to delays used
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
  // Normalizer (might remove / adjust if it doesn't really work)
  updateNormalizer(distR, distM, distL);
  distR = normalizedVals[0];
  distM = normalizedVals[1];
  distL  = normalizedVals[2];

  // Read QRE sensors 
  double whiteValR = analogRead(LDR_R);
  double whiteValL = analogRead(LDR_L);
  // Translate to detecting tape or not.
  bool whiteR = (LDR_DETECT_THRESHOLD > whiteValR);
  bool whiteL = (LDR_DETECT_THRESHOLD > whiteValL);

  debugger(distL, distM, distR, whiteValL, whiteValR);

  // Control
  if ((whiteR || whiteL) && !vertexMode) { // Detects edge, urgent escape
    escape(whiteR, whiteL);Serial.println("  ESCAPING");
  } else if (distR > DIST_DETECT_THRESHOLD && distL > DIST_DETECT_THRESHOLD && distM > DIST_DETECT_THRESHOLD_MIDDLE){ // Can't find opponent 
    if (!vertexMode) { // Spin Search
      spinSearch(); Serial.println("SPIN SEARCH");
    } else { // Vertex Search
      vertexSearch((whiteL || whiteR)); Serial.println("Vertex Search");
    }
  } else { // Found opponent
    spinMode = 0; vertexMode = 0;
    if (distR+12 < distL) { // opponent to left
      motors(motorSpeed(0.5), motorSpeed(1)); Serial.println("  LEFT");
    } else if (distR > distL+12) { // opponent to right
      motors(motorSpeed(1), motorSpeed(0.5)); Serial.println("  RIGHT");
    } else { // opponent in front
      motors(motorSpeed(1), motorSpeed(1)); Serial.println("  FWD");
    }
  }
}

// Measure Ultrasonic sensor (Middle)
  // Send out wave and get when it comes back or not
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

// I might remove this
// Normalizer
double pastDistVals[3][5] = {{0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}};
  // Sometimes the dist sensors give faulty values. In order to mitigate the sudden change, normalize values to past ~10 milliseconds (?)
void updateNormalizer (double distR, double distM, double distL) {
  // shift old vals
  for (int i = 0; i < 3; i++) {
    for (int j = 1; j < 5; j++) {
      pastDistVals[i][j-1] = pastDistVals[i][j];
    }
  }
  // add new value
  pastDistVals[0][4] = distR;
  pastDistVals[1][4] = distM;
  pastDistVals[2][4] = distL;
  // average out with past 5 vals and update normalizedVals
  for (int i = 0; i < 3; i++) { // i might make this more resistant to outliers
    double sum = 0;
    for (int j = 0; j < 5; j++) { // get sum of past vals
      sum += pastDistVals[i][j];
    }
    normalizedVals[i] = (sum)/5;
  }
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

// Spin Search for opponent bot
  // Spin in a circle
void spinSearch() {
  if (!spinMode) {
    spinMode = true;
    spinStartTime = millis();
  } else if (millis() - spinStartTime > 3000) { // span for too long
    spinMode = false;
    vertexMode = true;
  }
  motors(motorSpeed(-SPIN_SPEED), motorSpeed(SPIN_SPEED));
}

// Vertex Search for opponent bot
  // Move fwd until reach a vertex, then rotate some degrees then go to next vertex, repeat
void vertexSearch(bool vertex) {
  // 0 = forward
  // 1 = reverse from edge
  // 2 = rotate to point to next side
  static int state = 0;
  static unsigned long stateStart = 0;
  const unsigned long reverseTime = 500; // back off from edge
  const unsigned long rotateTime  = 1080; // rotation angle in terms of time
  unsigned long now = millis();

  if (state == 0) {  
    // Move forward until we hit edge
    motors(motorSpeed(0.7), motorSpeed(0.7));
    if (vertex) { // edge detected
      state = 1;
      stateStart = now;
    }
  } else if (state == 1) {  
    // Reverse slightly
    motors(motorSpeed(-0.7), motorSpeed(-0.7));
    if (now - stateStart >= reverseTime) {
      state = 2;
      stateStart = now;
    }
  } else if (state == 2) {  
    // Rotate in place
    motors(motorSpeed(-0.7), motorSpeed(0.7));
    if (now - stateStart >= rotateTime) {
      state = 0;
    }
  }
}

// Urgent, on tape
  // Move away from the tape
void escape(bool right, bool left) {
  motors(motorSpeed(-1), motorSpeed(-1));
}

// Translates percentage to PWM val
int motorSpeed(double percentage) {
  return (int)(percentage*(255));
}

// Convert MM to CM
double measurementCentimeters(int millimeter){
  return (millimeter)/10.0;
}

// Debugger String
void debugger(double distL, double distM, double distR, double whiteValL, double whiteValR){
  Serial.print("L: ");
  Serial.print(distL);
  Serial.print(" | M: ");
  Serial.print(distM);
  Serial.print(" | R: ");
  Serial.print(distR);
  Serial.print("||  L: ");
  Serial.print(whiteValL);
  Serial.print(" | R: ");
  Serial.print(whiteValR);
}