/*
 * GoBabyGo Car Controller
 * Writen by FIRST Team 1939 (The Kuhnigits)
 */

#include <Servo.h>


boolean SPEED_POTENTIOMETER = true;

// Invert one or two of the motors
boolean INVERT_1 = true;
boolean INVERT_2 = true;
//boolean INVERT_1 = false;
//boolean INVERT_2 = true;

// Constants
int SPEED_LIMIT = 256; // Between 0-512


int DEADBAND = 40; //150
int RAMPING = 1;
int REVERSE_PULSE    = 1000; // Talon SR is 1000
int FORWARD_PULSE    = 2000; // Talon SR is 2000

// Pins
int JOYSTICK_X = A0;
int JOYSTICK_Y = A1;
int MOTOR_1    = 10;
int MOTOR_2    = 11;
int SPEED_POT  = A3;
int invertStickX = -1; 
int invertStickY = 1; 

// Debug Over Serial - Requires a FTDI cable
boolean DEBUG = true;

// -----Don't Mess With Anything Past This Line-----

Servo motor1;
Servo motor2;


void setup() {
  pinMode(JOYSTICK_X, INPUT);
  pinMode(JOYSTICK_Y, INPUT);
  motor1.attach(MOTOR_1);
   motor2.attach(MOTOR_2);
  if(SPEED_POTENTIOMETER) pinMode(SPEED_POT, INPUT);
  
  if(DEBUG) Serial.begin(9600);
}

void loop() {
  //Read from the joystick
  int x = invertStickX * analogRead(JOYSTICK_X) + 1023;  
  int y =  invertStickY * analogRead(JOYSTICK_Y);
  debug("RawX:", x);
  debug("RawY:", y);

  //Zero values within deadband
  if(abs(512-x)<DEADBAND) x = 512;
  if(abs(512-y)<DEADBAND) y = 512;

  //Map values outside deadband to inside deadband
  if(x>512) x = map(x, 512+DEADBAND, 1023, 512, 1023);
  else if (x<512) x = map(x, 0, 512-DEADBAND, 0, 512);
  if(y>512) y = map(y, 512+DEADBAND, 1023, 512, 1023);
  else if(y<512) y = map(y, 0, 512-DEADBAND, 0, 512);

  //Establish a speed limit
  int limit = SPEED_LIMIT;
  if(SPEED_POTENTIOMETER) limit = map(analogRead(SPEED_POT), 0, 1023, 0, SPEED_LIMIT);
  debug("LIMIT", limit);

  //Map speeds to within speed limit
  x = map(x, 0, 1023, 512-limit, 512+limit);
  y = map(y, 0, 1023, 512-limit, 512+limit);
  debug("Xout:", x);
  debug("Yout:", y);
  
  int moveValue = 0;
  if(y>512) moveValue = y-512;
  else moveValue = -(512-y);
    
  int rotateValue = 0;
  if(x>512) rotateValue = x-512;
  else rotateValue = -(512-x);

  arcadeDrive(moveValue, rotateValue);
  
  debugF();
  delay(60); //Make loop run approximately 50hz
}

void arcadeDrive(int moveValue, int rotateValue) {
  int leftMotorSpeed = 0;
  int rightMotorSpeed = 0;
  if (moveValue > 0.0) {
      if (rotateValue > 0.0) {
        leftMotorSpeed = moveValue - rotateValue;
        rightMotorSpeed = max(moveValue, rotateValue);
      } else {
        leftMotorSpeed = max(moveValue, -rotateValue);
        rightMotorSpeed = moveValue + rotateValue;
      }
    } else {
      if (rotateValue > 0.0) {
        leftMotorSpeed = -max(-moveValue, rotateValue);
        rightMotorSpeed = moveValue + rotateValue;
      } else {
        leftMotorSpeed = moveValue - rotateValue;
        rightMotorSpeed = -max(-moveValue, -rotateValue);
      }
    }
    drive(map(leftMotorSpeed, -512, 512, 0, 1023), map(rightMotorSpeed, -512, 512, 0, 1023));
}

int prevLeft = 500;
int prevRight = 500;

void drive(int left, int right){
  int speed1 = map(left, 0, 1023, 0, FORWARD_PULSE-REVERSE_PULSE);
  if(speed1>prevLeft+RAMPING) speed1=speed1+RAMPING;
  else if(speed1<prevLeft-RAMPING) speed1=speed1-RAMPING;
  if(INVERT_1) motor1.write(FORWARD_PULSE-speed1);
  else motor1.write(REVERSE_PULSE+speed1);
  prevLeft = speed1;
  
  int speed2 = map(right, 0, 1023, 0, FORWARD_PULSE-REVERSE_PULSE);
  if(speed2>prevLeft+RAMPING) speed2=speed2+RAMPING;
  else if(speed2<prevLeft-RAMPING) speed2=speed2-RAMPING;
  if(INVERT_2) motor2.write(FORWARD_PULSE-speed2);
  else motor2.write(REVERSE_PULSE+speed2);
  prevRight = speed2;
}

void debug(String s, int value){
  if(DEBUG){
    Serial.print(s);
    Serial.print(": ");
    Serial.print(value);
  }
}
void debugF(){
  if(DEBUG){
    Serial.println("");
  }
}
