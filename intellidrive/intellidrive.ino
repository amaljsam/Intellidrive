#include <Wire.h>
#include <Servo.h>

// Servo Motor
Servo steeringServo;

// Servo Angle Definitions
const int SERVO_PIN = 10;  
const int CENTER_ANGLE = 115;  
const int LEFT_ANGLE = 155;   
const int RIGHT_ANGLE = 75;  

// Line Sensor Array
const int NUM_SENSORS = 4;  
const int lineSensorPins[] = {A0, A1, A2, A3}; 

// Ultrasonic Sensor
#define TRIG_PIN_FRONT 11
#define ECHO_PIN_FRONT 12
#define TRIG_PIN_RIGHT 13
#define ECHO_PIN_RIGHT 2
#define TRIG_PIN_LEFT 3
#define ECHO_PIN_LEFT 1  

// Motor Driver Pins
#define MOTOR_A_IN1 4
#define MOTOR_A_IN2 5
#define MOTOR_B_IN3 6
#define MOTOR_B_IN4 7
#define ENA 8
#define ENB 9

// Distance Thresholds
const int FRONT_OBSTACLE_DISTANCE = 60;  
const int SIDE_OBSTACLE_DISTANCE = 30;   
const int LINE_THRESHOLD = 500;  

// Flags
int direction = 0;  
int lastTurn = 0;   

void setup() {
  Serial.begin(9600);

  // Initialize Servo
  steeringServo.attach(SERVO_PIN);
  setServoAngle(CENTER_ANGLE);

  // Ultrasonic Sensor Pins
  pinMode(TRIG_PIN_FRONT, OUTPUT);
  pinMode(ECHO_PIN_FRONT, INPUT);
  pinMode(TRIG_PIN_RIGHT, OUTPUT);
  pinMode(ECHO_PIN_RIGHT, INPUT);
  pinMode(TRIG_PIN_LEFT, OUTPUT);
  pinMode(ECHO_PIN_LEFT, INPUT);

  // Motor Driver Pins
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(MOTOR_B_IN3, OUTPUT);
  pinMode(MOTOR_B_IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Line Sensor Pins
  for (int i = 0; i < NUM_SENSORS; i++) {
    pinMode(lineSensorPins[i], INPUT);
  }
}

void loop() {
  automaticControl();
}

// Function to control the servo angle
void setServoAngle(int angle) {
  steeringServo.write(angle);
}

void moveForward() {
  digitalWrite(MOTOR_A_IN1, HIGH);
  digitalWrite(MOTOR_A_IN2, LOW);
  digitalWrite(MOTOR_B_IN3, HIGH);
  digitalWrite(MOTOR_B_IN4, LOW);
  analogWrite(ENA, 200);
  analogWrite(ENB, 200);
}

void moveBackward() {
  digitalWrite(MOTOR_A_IN1, LOW);
  digitalWrite(MOTOR_A_IN2, HIGH);
  digitalWrite(MOTOR_B_IN3, LOW);
  digitalWrite(MOTOR_B_IN4, HIGH);
  analogWrite(ENA, 200);
  analogWrite(ENB, 200);
}

void turnLeft() {
  setServoAngle(LEFT_ANGLE);
  moveForward();
  delay(1000);
  stopCar();
}

void turnRight() {
  setServoAngle(RIGHT_ANGLE);
  moveForward();
  delay(1000);
  stopCar();
}

void stopCar() {
  digitalWrite(MOTOR_A_IN1, LOW);
  digitalWrite(MOTOR_A_IN2, LOW);
  digitalWrite(MOTOR_B_IN3, LOW);
  digitalWrite(MOTOR_B_IN4, LOW);
}

// Avoid obstacle by turning right
void avoidObstacleRight() {
  stopCar();
  delay(500);  
  setServoAngle(RIGHT_ANGLE);
  moveForward();
  delay(1000);  
  stopCar();
  setServoAngle(LEFT_ANGLE);
  moveForward();
  delay(1000);  
  stopCar();
  setServoAngle(CENTER_ANGLE);
  moveForward();
  delay(1000);  
  stopCar();
  lastTurn=1;
}

void avoidObstacleLeft() {
  stopCar();
  delay(500);  
  setServoAngle(LEFT_ANGLE);
  moveForward();
  delay(1000);  
  stopCar();
  setServoAngle(RIGHT_ANGLE);
  moveForward();
  delay(1000);  
  stopCar();
  setServoAngle(CENTER_ANGLE);
  moveForward();
  delay(1000);  
  stopCar();
  lastTurn=0;
}

// Automatic control function
void automaticControl() {
  long frontDistance = getUltrasonicDistance(TRIG_PIN_FRONT, ECHO_PIN_FRONT);
  long rightDistance = getUltrasonicDistance(TRIG_PIN_RIGHT, ECHO_PIN_RIGHT);
  long leftDistance = getUltrasonicDistance(TRIG_PIN_LEFT, ECHO_PIN_LEFT);

  Serial.print("Front: ");
  Serial.print(frontDistance);
  Serial.print(" cm, Right: ");
  Serial.print(rightDistance);
  Serial.print(" cm, Left: ");
  Serial.println(leftDistance);

  // Obstacle Avoidance
 if (frontDistance > FRONT_OBSTACLE_DISTANCE) {
    
   
    followLine();
} 
else { // Obstacle detected in front
    stopCar();
    delay(300);

    if (rightDistance > SIDE_OBSTACLE_DISTANCE && frontDistance < FRONT_OBSTACLE_DISTANCE && lastTurn == 0) { 
        avoidObstacleRight();
        setServoAngle(CENTER_ANGLE);
    } 
    else if (leftDistance > SIDE_OBSTACLE_DISTANCE && lastTurn == 1) { 
        avoidObstacleLeft(); 
        setServoAngle(CENTER_ANGLE);
    } 
    else if ((frontDistance < FRONT_OBSTACLE_DISTANCE && rightDistance < SIDE_OBSTACLE_DISTANCE && lastTurn == 0) || 
             (frontDistance < FRONT_OBSTACLE_DISTANCE && leftDistance < SIDE_OBSTACLE_DISTANCE && lastTurn == 1)) {
        setServoAngle(CENTER_ANGLE);
        stopCar();
        delay(1000);
    }
    else { 
        stopCar();
    }
}


 
  
}


// Get distance details
long getUltrasonicDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  long distance = duration * 0.034 / 2;  
  return distance;
}

void followLine() {
  int sensorValues[NUM_SENSORS];
  int leftCount = 0, rightCount = 0;

  // Read line sensor values
  for (int i = 0; i < NUM_SENSORS; i++) {
    sensorValues[i] = analogRead(lineSensorPins[i]) < LINE_THRESHOLD ? 1 : 0;
  }

  // Determine position based on sensor activation
  leftCount = sensorValues[0] + sensorValues[1];  // Left sensors
  rightCount = sensorValues[2] + sensorValues[3]; // Right sensors

  if (leftCount > rightCount) {
    setServoAngle(RIGHT_ANGLE);  // Turn right
  } else if (rightCount > leftCount) {
    setServoAngle(LEFT_ANGLE); // Turn left
  } else {
    setServoAngle(CENTER_ANGLE); // Move straight
  }

  moveForward();
}


void adjustMotorSpeed(bool avoidObstacle) {
  if (avoidObstacle) {
    analogWrite(ENA, 150);
    analogWrite(ENB, 150);  
  } else {
    analogWrite(ENA, 200);
    analogWrite(ENB, 200);  
  }
}
