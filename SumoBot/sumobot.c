// Sehan M., Elya K., Vraj P.
// TERM - Mr. Wong
// 2026-02-XX
// Sumo Bot

// Motors
#define R_MOTOR_PWM 3 // PWMA
#define R_MOTOR_P1 4 // AIN1
#define R_MOTOR_P2 5 // AIN2
#define L_MOTOR_PWM 6 // PWMB
#define L_MOTOR_P1 7 // BIN1
#define L_MOTOR_P2 8 // BIN2
#define MOTOR_STBY 9 // STBY

// Ground Sensors
#define LDR_F A0
#define LDR_B A1

// Dist Sensors
#define DIST_F A2
#define DIST_B A3

#define BASE_SPEED 150 // 1 - 255
#define DETECT_THRESHOLD 300
#define LDR_CONSTRAINT 600

void setup() {
  pinMode(R_MOTOR_P1, OUTPUT);
  pinMode(R_MOTOR_P2, OUTPUT);
  pinMode(L_MOTOR_P1, OUTPUT);
  pinMode(L_MOTOR_P2, OUTPUT);

  pinMode(R_MOTOR_PWM, OUTPUT);
  pinMode(L_MOTOR_PWM, OUTPUT);
  pinMode(MOTOR_STBY, OUTPUT);

  digitalWrite(MOTOR_STBY, HIGH);

  Serial.begin(9600);
}

void loop() {
  int distF = analogRead(DIST_F);
  int distB = analogRead(DIST_B);
  
  int ldrF = analogRead(LDR_F);
  int ldrB = analogRead(LDR_B);

  Serial.print("Dist F: ");
  Serial.print(distF);
  Serial.print("  Dist B: ");
  Serial.print(distB);
  Serial.print("  LDR F: ");
  Serial.print(ldrF);
  Serial.print("  LDR B: ");
  Serial.println(ldrB);

  bool frontWhite = ldrF > LDR_CONSTRAINT;
  bool backWhite  = ldrB > LDR_CONSTRAINT;


  if (frontWhite && backWhite) { // pretty much lost senario
    motors(BASE_SPEED, -BASE_SPEED);
  } else if (frontWhite) {
    escapeB();
  } else if (backWhite) {
    escapeF();
  } else {
    if (distF > DETECT_THRESHOLD || distB > DETECT_THRESHOLD) {
      stopMotors();
    } else {
      spin();
    }
  }
  delay(50);
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