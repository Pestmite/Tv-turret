// || Libraries //
#include <Arduino.h>
#include <Servo.h>
#include "PinDefinitionsAndMore.h" // Define macros for input and output pin etc.
#include <IRremote.hpp>

#define DECODE_NEC  //defines the type of IR transmission to decode based on the remote. See IRremote library for examples on how to decode other types of remote


// add tv remote by pressing it in front of turret and checking the hex code with serial monitor

//defines the specific command code for each button on the remote
#define power 0x8
#define ok 0x5A
#define up 0x57
#define down 0x56
#define left 0x58
#define right 0x59
#define exit 0x4D
#define cmd1 0x11
#define cmd2 0x12
#define cmd3 0x13
#define cmd4 0x14
#define cmd5 0x15
#define cmd6 0x16
#define cmd7 0x17
#define cmd8 0x18
#define cmd9 0x19
#define cmd0 0x10
#define vidaaTV 0xEA
#define vidaaFree 0xC2
#define netflix 0x4C
#define prime 0x47
#define volumeDown 0x3
#define volumeUp 0x2

// || Global Variables //
Servo yawServo; //names the servo responsible for YAW rotation, 360 spin around the base
Servo pitchServo; //names the servo responsible for PITCH rotation, up and down tilt
Servo rollServo; //names the servo responsible for ROLL rotation, spins the barrel to fire darts


int yawServoVal; //initialize variables to store the current value of each servo
int pitchServoVal = 80;
int rollServoVal;


int pitchMoveSpeed = 8; //this variable is the angle added to the pitch servo to control how quickly the PITCH servo moves - try values between 3 and 10
int yawMoveSpeed = 90; //this variable is the speed controller for the continuous movement of the YAW servo motor. It is added or subtracted from the yawStopSpeed, so 0 would mean full speed rotation in one direction, and 180 means full rotation in the other. Try values between 10 and 90;
int yawStopSpeed = 90; //value to stop the yaw motor - keep this at 90
int rollMoveSpeed = 90; //this variable is the speed controller for the continuous movement of the ROLL servo motor. It is added or subtracted from the rollStopSpeed, so 0 would mean full speed rotation in one direction, and 180 means full rotation in the other. Keep this at 90 for best performance / highest torque from the roll motor when firing.
int rollStopSpeed = 90; //value to stop the roll motor - keep this at 90


int yawPrecision = 50; // this variable represents the time in milliseconds that the YAW motor will remain at it's set movement speed. Try values between 50 and 500 to start (500 milliseconds = 1/2 second)
int rollPrecision = 158; // this variable represents the time in milliseconds that the ROLL motor with remain at it's set movement speed. If this ROLL motor is spinning more or less than 1/6th of a rotation when firing a single dart (one call of the fire(); command) you can try adjusting this value down or up slightly, but it should remain around the stock value (160ish) for best results.


int pitchMax = 175; // this sets the maximum angle of the pitch servo to prevent it from crashing, it should remain below 180, and be greater than the pitchMin
int pitchMin = 10; // this sets the minimum angle of the pitch servo to prevent it from crashing, it should remain above 0, and be less than the pitchMax


void shakeHeadYes(int moves = 3);
void shakeHeadNo(int moves = 3);


// || S E T U P  L O O P //
void setup() {
  Serial.begin(9600); // initializes the Serial communication between the computer and the microcontroller


  yawServo.attach(10); //attach YAW servo to pin 10
  pitchServo.attach(11); //attach PITCH servo to pin 11
  rollServo.attach(12); //attach ROLL servo to pin 12


  // Just to know which program is running on my microcontroller
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));


  // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
  IrReceiver.begin(9, ENABLE_LED_FEEDBACK);


  Serial.print(F("Ready to receive IR signals of protocols: "));
  printActiveIRProtocols(&Serial);
  Serial.println(F("at pin " STR(9)));




  homeServos(); //set servo motors to home position
}


// || M A I N  L O O P  //
void loop() {
  // Check if received data is available and if yes, try to decode it.


  if (IrReceiver.decode()) {
      // Print a short summary of received data


      IrReceiver.printIRResultShort(&Serial);
      IrReceiver.printIRSendUsage(&Serial);
      if (IrReceiver.decodedIRData.protocol == UNKNOWN) { //command garbled or not recognized
          Serial.println(F("Received noise or an unknown (or not yet enabled) protocol - if you wish to add this command, define it at the top of the file with the hex code printed below (ex: 0x8)"));
          // We have an unknown protocol here, print more info
          IrReceiver.printIRResultRawFormatted(&Serial, true);
      }
      Serial.println();


    
      // !!!Important!!! Enable receiving of the next value,
      // since receiving has stopped after the end of the current received data packet.


      IrReceiver.resume(); // Enable receiving of the next value




      // Finally, check the received data and perform actions according to the received command


      switch(IrReceiver.decodedIRData.command){ //this is where the commands are handled
          case netflix: // fire all
            fireAll();
            fireAll();
            break;
          
          case prime: // fire all
            fireAll();
            fireAll();
            break;

          case vidaaTV:
            shakeHeadYes();
            delay(50);
            break;

          case vidaaFree:
            shakeHeadNo();
            delay(50);
            break;

          case volumeUp:
            shakeHeadYes(6);
            delay(50);
            break;

          case volumeDown:
            shakeHeadNo(6);
            delay(50);
            break;
      }
  }
  delay(5);
}


void shakeHeadYes(int moves = 3) {
  Serial.println("YES");
  int startAngle = pitchServoVal; // Current position of the pitch servo
  int lastAngle = pitchServoVal;
  int nodAngle = startAngle + 20; // Angle for nodding motion


  for (int i = 0; i < moves; i++) { // Repeat nodding motion three times
    // Nod up
    for (int angle = startAngle; angle <= nodAngle; angle++) {
        pitchServo.write(angle);
        delay(7); // Adjust delay for smoother motion
    }
    delay(50); // Pause at nodding position
    // Nod down
    for (int angle = nodAngle; angle >= startAngle; angle--) {
        pitchServo.write(angle);
        delay(7); // Adjust delay for smoother motion
    }
    delay(50); // Pause at starting position
  }
}


void shakeHeadNo(int moves = 3) {
  Serial.println("NO");
  int startAngle = pitchServoVal; // Current position of the pitch servo
  int lastAngle = pitchServoVal;
  int nodAngle = startAngle + 60; // Angle for nodding motion


  for (int i = 0; i < moves; i++) { // Repeat nodding motion three times
    // rotate right, stop, then rotate left, stop
    yawServo.write(140);
    delay(190); // Adjust delay for smoother motion
    yawServo.write(yawStopSpeed);
    delay(50);
    yawServo.write(40);
    delay(190); // Adjust delay for smoother motion
    yawServo.write(yawStopSpeed);
    delay(50); // Pause at starting position
  }
}


void leftMove(int moves){
   for (int i = 0; i < moves; i++){
      yawServo.write(yawStopSpeed + yawMoveSpeed); // adding the servo speed = 180 (full counterclockwise rotation speed)
      delay(yawPrecision); // stay rotating for a certain number of milliseconds
      yawServo.write(yawStopSpeed); // stop rotating
      delay(5); //delay for smoothness
      Serial.println("LEFT");
 }
}


void rightMove(int moves){ // function to move right
 for (int i = 0; i < moves; i++){
    yawServo.write(yawStopSpeed - yawMoveSpeed); //subtracting the servo speed = 0 (full clockwise rotation speed)
    delay(yawPrecision);
    yawServo.write(yawStopSpeed);
    delay(5);
    Serial.println("RIGHT");
 }
}


void upMove(int moves){
 for (int i = 0; i < moves; i++){
    if(pitchServoVal > pitchMin){//make sure the servo is within rotation limits (greater than 10 degrees by default)
      pitchServoVal = pitchServoVal - pitchMoveSpeed; //decrement the current angle and update
      pitchServo.write(pitchServoVal);
      delay(50);
      Serial.println("UP");
    }
 }
}


void downMove (int moves){
 for (int i = 0; i < moves; i++){
    if(pitchServoVal < pitchMax){ //make sure the servo is within rotation limits (less than 175 degrees by default)
      pitchServoVal = pitchServoVal + pitchMoveSpeed;//increment the current angle and update
      pitchServo.write(pitchServoVal);
      delay(50);
      Serial.println("DOWN");
    }
 }
}


// Fires a single dart
void fire() { //function for firing a single dart
    rollServo.write(rollStopSpeed + rollMoveSpeed);//start rotating the servo
    delay(rollPrecision);//time for approximately 60 degrees of rotation
    rollServo.write(rollStopSpeed);//stop rotating the servo
    delay(5); //delay for smoothness
    Serial.println("FIRING");
}


// Fire every dart
void fireAll() {
  rollServo.write(rollStopSpeed + rollMoveSpeed);//start rotating the servo
  delay(rollPrecision * 6); //time for 360 degrees of rotation
  rollServo.write(rollStopSpeed);//stop rotating the servo
  delay(5); // delay for smoothness
  Serial.println("FIRING ALL");
}


// sets servo to starting position
void homeServos(){
  yawServo.write(yawStopSpeed); //setup YAW servo to be STOPPED (90)
  delay(20);
  rollServo.write(rollStopSpeed); //setup ROLL servo to be STOPPED (90)
  delay(100);
  pitchServo.write(100); //set PITCH servo to 100 degree position
  delay(100);
  pitchServoVal = 100; // store the pitch servo value
  Serial.println("HOMING");
}