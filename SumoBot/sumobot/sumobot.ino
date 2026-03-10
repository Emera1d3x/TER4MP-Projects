// Temporary version for broken bot (one of the IR sensors and both of the VL53L0X got cooked)

// Sehan M., Elya K., Vraj P.
// TERM - Mr. Wong
// 2026-03-09
// Sumo Bot

// Imports
#include <Wire.h>
#include <time.h>
#include <Adafruit_VL53L0X.h>

// Motors
  // Right Motor
#define R_MOTOR_PWM 5 // PWMA
#define R_MOTOR_P1 8 // AIN1
#define R_MOTOR_P2 7 // AIN2
  // Left Motor
#define L_MOTOR_PWM 6 // PWMB
#define L_MOTOR_P1 10 // BIN1
#define L_MOTOR_P2 9 // BIN2

// Ground Sensors (QRE1113 IR Sensor)
  // Right Ground Sensor
#define IR_R A0
  // Left Ground Sensor
#define IR_L A1

// Distance Sensors (VL53L0X Dual Sensor & Ultrasonic Sensor)
  // Dist Sensor (VL53L0X) Objects
//Adafruit_VL53L0X distSensorR = Adafruit_VL53L0X();
//Adafruit_VL53L0X distSensorL = Adafruit_VL53L0X();
  // SHT pins
#define DISTR_SHT 2
#define DISTL_SHT 3
  // Memory Allocation - required for multi-sensor setup
#define DISTR_ADDRESS 0x30
#define DISTL_ADDRESS 0x31
  // Ultrasonic sensor pins
#define ULTRA_TRIG 11
#define ULTRA_ECHO 12

// Constants, for calibration
#define SPIN_SPEED 0.4 // Spin Speed Percentage
#define DIST_DETECT_THRESHOLD 18 // Recognize object when values read <
#define DIST_DETECT_THRESHOLD_MIDDLE 35 // Recognize object when values read <
#define IR_DETECT_THRESHOLD 110 // Recognize white when values read <

// Search variables
  // Abandon basic spin search after some time for a vertex search, prevents stalemates 
bool spinMode = false;
bool vertexMode = false;
unsigned long spinStartTime = 0;

// Normalized Values
  // Smoothens dist values, reduces jitters, and mitigates random uncontrollable reading errors 
double normalizedVals[3] = {0.0, 0.0, 0.0};

// Setup Dist Sensors
void setDistSensors(){
  //setVL52L0X();
  setUltraSonic();
}

// Setup (DIST) VL52L0X Sensors
  // Weird method required when setting multiple VL52L0X
  // Start with both sensors off, then restart one sensor and allocate to one memory address, then restart the other at another memoory address via XSHUT pins
/*void setVL52L0X() {
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
}*/

// Setup (DIST) UltraSonic Sensor
  // Trig Pin sends beam
  // Echo Pin detects beam
void setUltraSonic(){
  pinMode(ULTRA_TRIG, OUTPUT);
  pinMode(ULTRA_ECHO, INPUT);
}

// Setup (IR for sensing border) QRE
void setQRE(){
  pinMode(IR_R, INPUT);
  pinMode(IR_L, INPUT);
}

// Setup Motors
  // Regular and PWM pins for custom speed
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
  // Setup components
  setDistSensors();
  setMotors();
  setQRE();
  Serial.println("Setup Finished");
  delay(1);
  //motors(motorSpeed(1), motorSpeed(1)); // Move forward away from opponent for some time
  //delay(500000);
}

// Loop
  // Constantly called method, around 16 MHz (?) but slowed due to delays used
void loop() {
  // Read VL53L0X sensors for current information
  /*VL53L0X_RangingMeasurementData_t measureDISTR;
  VL53L0X_RangingMeasurementData_t measureDISTL;
  distSensorR.rangingTest(&measureDISTR, false);
  distSensorL.rangingTest(&measureDISTL, false);
  // Get distances for all sensors
  double distR = measurementCentimeters(measureDISTR.RangeMilliMeter);
  double distL = measurementCentimeters(measureDISTL.RangeMilliMeter);*/
  double distM = measureUltraSonic();
  double distR = 0.0;
  double distL = 0.0;
  // Normalize dist values to be smoother
  updateNormalizer(distR, distM, distL);
  distR = normalizedVals[0];
  distM = normalizedVals[1];
  distL = normalizedVals[2];

  // Read QRE sensors 
  double whiteValR = analogRead(IR_R);
  double whiteValL = analogRead(IR_L);
  // Translate to detecting border or not.
  bool whiteR = (IR_DETECT_THRESHOLD > whiteValR);
  bool whiteL = (IR_DETECT_THRESHOLD > whiteValL);
  whiteL = false;
  if (distM == 0.0){
    distM = 100.0;
  }
  debugger(distL, distM, distR, whiteValL, whiteValR);

  
  // Controls
  /*
  if ((whiteR || whiteL) && !vertexMode) { // Detects edge, urgent escape
    escape(whiteR, whiteL);Serial.println("  ESCAPING");
  } else if (distR > DIST_DETECT_THRESHOLD && distL > DIST_DETECT_THRESHOLD && distM > DIST_DETECT_THRESHOLD_MIDDLE){ // Can't find opponent 
    if (!vertexMode) { // Spin Search
      spinSearch(); Serial.println("SPIN SEARCH");
    } else { // Vertex Search, happens after some time
      vertexSearch((whiteL || whiteR)); Serial.println("Vertex Search");
    }
  } else { // Found opponent
    spinMode = 0; vertexMode = 0;
    if (distR+12 < distL) { // opponent to left
      motors(motorSpeed(0.5), motorSpeed(1)); Serial.println("  LEFT"); // Adjust lefet
    } else if (distR > distL+12) { // opponent to right
      motors(motorSpeed(1), motorSpeed(0.5)); Serial.println("  RIGHT"); // Adjust right
    } else { // opponent in front
      motors(motorSpeed(1), motorSpeed(1)); Serial.println("  FWD"); // Move fwd (attack)
    }
  }
  */
  if ((whiteR || whiteL) && !vertexMode) { // Detects edge, urgent escape
    escape(whiteR, whiteL);Serial.println("  ESCAPING");
  } else if (distM > DIST_DETECT_THRESHOLD_MIDDLE){ // Can't find opponent 
    if (!vertexMode) { // Spin Search
      spinSearch(); Serial.println(" SPIN SEARCH");
    } else { // Vertex Search, happens after some time
      vertexSearch((whiteL || whiteR)); Serial.println(" Vertex Search");
    }
  } else { // Found opponent
    spinMode = 0; vertexMode = 0;
    motors(motorSpeed(1), motorSpeed(1)); Serial.println("  FWD"); // Move fwd (attack)
  }
}

// Measure Ultrasonic sensor (Middle)
  // Sends out wave and gets information on when it comes back if it does
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

// Normalizer
  // Smoothen values
double pastDistVals[3][5] = {{0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0}};
  // Sometimes the dist sensors give faulty values. In order to mitigate the sudden change, normalize values to past ~10 milliseconds (?)
void updateNormalizer (double distR, double distM, double distL) {
  // Shift old values
  for (int i = 0; i < 3; i++) {
    for (int j = 1; j < 5; j++) {
      pastDistVals[i][j-1] = pastDistVals[i][j];
    }
  }
  // Add new values
  pastDistVals[0][4] = distR;
  pastDistVals[1][4] = distM;
  pastDistVals[2][4] = distL;
  // Average out with past 5 vals and update normalizedVals
  for (int i = 0; i < 3; i++) {
    double sum = 0;
    for (int j = 0; j < 5; j++) { // Get sum of past vals
      sum += pastDistVals[i][j];
    }
    normalizedVals[i] = (sum)/5; // Put averaged value into normalizedVals[]
  }
}

// Move Motors
  // Move motors right and left. Positive is forward and negative is backwards.
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

// Spin Search
  // Spin in place to find opponent
void spinSearch() {
  if (!spinMode) {
    spinMode = true;
    spinStartTime = millis();
  } else if (millis() - spinStartTime > 3000) { // If span for too long, abandon spin search and do vertex search process
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
    // Move forward until detects edge
    motors(motorSpeed(1), motorSpeed(1));
    if (vertex) { // Edge detected
      state = 1;
      stateStart = now;
    }
  } else if (state == 1) {
    // Reverse slightly
    motors(motorSpeed(-1), motorSpeed(-1));
    if (now - stateStart >= reverseTime) {
      state = 2;
      stateStart = now;
    }
  } else if (state == 2) {  
    // Rotate in place for some time to point itself to next edge
    motors(motorSpeed(-0.7), motorSpeed(0.7));
    if (now - stateStart >= rotateTime) {
      state = 0;
    }
  }
}

// Urgent, on tape
  // Move away from the tape (move backwards)
void escape(bool right, bool left) {
  motors(motorSpeed(-1), motorSpeed(-1));
}

// Translates percentage (1.0 to 0 to -1.0) to PWM val
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