#include <Adafruit_ATParser.h>
#include <Adafruit_BLE.h>
#include <Adafruit_BLEBattery.h>
#include <Adafruit_BLEEddystone.h>
#include <Adafruit_BLEGatt.h>
#include <Adafruit_BLEMIDI.h>
#include <Adafruit_BluefruitLE_SPI.h>
#include <Adafruit_BluefruitLE_UART.h>
#include "BluefruitConfig.h"

/*
 * GoBabyGo Car Controller for Power Wheels Wild Thing Modifications
 *Original by FIRST Team 1939 (The Kuhnigits) 
 *Modified By Rogue Robotics Team 2987 
 *Full guide available at https://www.team2987.com/gobabygo
 */

#include <Servo.h>
#include <SPI.h>
#include <string.h>
#define BLUEFRUIT_SPI_CS               8
#define BLUEFRUIT_SPI_IRQ              7
#define BLUEFRUIT_SPI_RST              4 

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

enum controlMode {childControl, parentControl}; 
controlMode driveMode = childControl;
boolean SPEED_POTENTIOMETER = false;

// Invert one or two of the motors
boolean INVERT_1 = true;
boolean INVERT_2 = false;
//boolean INVERT_1 = false;
//boolean INVERT_2 = true;

// Constants
int SPEED_LIMIT = 256; // Between 0-512, recomended 256 for talon SR's and wild thing motors 

Servo servo; 
#define FACTORYRESET_ENABLE         0
    #define MINIMUM_FIRMWARE_VERSION    "0.6.6"
    #define MODE_LED_BEHAVIOUR          "MODE"
int DEADBAND = 40; //150
int RAMPING = 1;
int REVERSE_PULSE    = 1000; // Talon SR is 1000
int FORWARD_PULSE    = 2000; // Talon SR is 2000

// Pins
int JOYSTICK_X = A0;
int JOYSTICK_Y = A1;
int MOTOR_1    = 10;
int MOTOR_2    = 3;
int SPEED_POT  = A3;
int invertStickX = -1; 
int invertStickY = 1; 
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
float parsefloat(uint8_t *buffer);
void printHex(const uint8_t * data, const uint32_t numBytes);

// Debug Over Serial - Requires a FTDI cable
boolean DEBUG = false;
extern uint8_t packetbuffer[];
// -----Don't Mess With Anything Past This Line-----

Servo motor1;
Servo motor2;


void setup() {
  while (!Serial);  // required for Flora & Micro
  delay(500);
  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit App Controller Example"));
  Serial.println(F("-----------------------------------------"));
  Serial.print(F("Initialising the Bluefruit LE module: "));
  //servo.attach(3);
  pinMode(JOYSTICK_X, INPUT);
  pinMode(JOYSTICK_Y, INPUT);
  motor1.attach(MOTOR_1);
  motor2.attach(MOTOR_2);
  if(SPEED_POTENTIOMETER) pinMode(SPEED_POT, INPUT);
  
  if(DEBUG) Serial.begin(9600);
  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  ble.echo(false);
  Serial.println("Requesting Bluefruit info:"); 
  ble.info(); 
  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in Controller mode"));
  Serial.println(F("Then activate/use the sensors, color picker, game controller, etc!"));
  Serial.println();
  ble.verbose(false);  // debug info is a little annoying after this point!
//  while (! ble.isConnected()) {
//      delay(500);
//  }

  Serial.println(F("******************************"));

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  // Set Bluefruit to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("******************************"));

}

void loop() {
   if(ble.isConnected()){
    Serial.print("Before len"); 
    Serial.println(millis()); 
    uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
    Serial.print("After len"); 
    Serial.println(millis());
    //if (len == 0) return;
    Serial.print(packetbuffer[0]);
    Serial.print(" ");
    Serial.print(packetbuffer[1]);
    Serial.print(packetbuffer[2]);
    Serial.print(packetbuffer[3]);

    if (!(len == 0) && packetbuffer[1] == 'B') {
      uint8_t buttnum = packetbuffer[2] - '0';
      boolean pressed = packetbuffer[3] - '0';
      
    
      // Changes the drive mode if button 1 is pressed 
      if (pressed) {
      
        if(buttnum == 1){ 
          if(driveMode == childControl){
            driveMode = parentControl; 
          }
          else if(driveMode == parentControl){
            driveMode = childControl; 
          }
        }
       }
      }

    // Control logic for driving with a phone. Makes the chair go at 1/4 the speed limit. 
   if(!(len == 0) && driveMode == parentControl){
    if (packetbuffer[1] == 'B') {
      uint8_t buttnum = packetbuffer[2] - '0';
      boolean pressed = packetbuffer[3] - '0';
      if(pressed){
        if(buttnum == 8){
          arcadeDrive(SPEED_LIMIT/3, 0); 
        }
        else if(buttnum == 7){
          arcadeDrive(-SPEED_LIMIT/3, 0); 
        }
        if(buttnum == 6){
          arcadeDrive(0, SPEED_LIMIT/3); 
        }
        else if(buttnum == 5){
          arcadeDrive(0,-SPEED_LIMIT/3); 
        }
      }
      else{
        arcadeDrive(0,0); 
      }
     }
    }
 
  
    }

    //if(!ble.isConnected()) driveMode = childControl; 
   
    
  

  //Control logic for regular drive 
  //Serial.print("After Blue tooth"); 
 // Serial.println(millis());
  if(driveMode == childControl){
    
    Serial.println("Child Control Mode");
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
