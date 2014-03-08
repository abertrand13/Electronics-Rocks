
// ===========================
// MASTER CODE - Creative Name for our Product
// Authors: EE 27N (names in alphabetical order eventually)
//
// Copyleft
// Open source
// Everything else that indicates that you should totally download this.
//
// I2C device class (I2Cdev) demonstration Arduino sketch for MPU6050 class using DMP (MotionApps v2.0)
// 6/21/2012 by Jeff Rowberg <jeff@rowberg.net>
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//      2013-05-08 - added seamless Fastwire support
//                 - added note about gyro calibration
//      2012-06-21 - added note about Arduino 1.0.1 + Leonardo compatibility error
//      2012-06-20 - improved FIFO overflow handling and simplified read process
//      2012-06-19 - completely rearranged DMP initialization code and simplification
//      2012-06-13 - pull gyro and accel data from FIFO packet instead of reading directly
//      2012-06-09 - fix broken FIFO read sequence and change interrupt detection to RISING
//      2012-06-05 - add gravity-compensated initial reference frame acceleration output
//                 - add 3D math helper file to DMP6 example sketch
//                 - add Euler output and Yaw/Pitch/Roll output formats
//      2012-06-04 - remove accel offset clearing for better results (thanks Sungon Lee)
//      2012-06-01 - fixed gyro sensitivity to be 2000 deg/sec instead of 250
//      2012-05-30 - basic DMP initialization working

/* ============================================
 I2Cdev device library code is placed under the MIT license
 Copyright (c) 2012 Jeff Rowberg
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ===============================================
 */

// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"

#include "MPU6050_6Axis_MotionApps20.h"
//#include "MPU6050.h" // not necessary if using MotionApps include file
#include "math.h"
//for ^ function

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for SparkFun breakout and InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 mpu;
//MPU6050 mpu(0x69); // <-- use for AD0 high

/* =========================================================================
 NOTE: In addition to connection 3.3v, GND, SDA, and SCL, this sketch
 depends on the MPU-6050's INT pin being connected to the Arduino's
 external interrupt #0 pin. On the Arduino Uno and Mega 2560, this is
 digital I/O pin 2.
 * ========================================================================= */

/* =========================================================================
 NOTE: Arduino v1.0.1 with the Leonardo board generates a compile error
 when using Serial.write(buf, len). The Teapot output uses this method.
 The solution requires a modification to the Arduino USBAPI.h file, which
 is fortunately simple, but annoying. This will be fixed in the next IDE
 release. For more info, see these links:
 
 http://arduino.cc/forum/index.php/topic,109987.0.html
 http://code.google.com/p/arduino/issues/detail?id=958
 * ========================================================================= */

#define LED_PIN 13 // (Arduino is 13, Teensy is 11, Teensy++ is 6)
bool blinkState = false;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

// packet structure for InvenSense teapot demo
uint8_t teapotPacket[14] = { 
  '$', 0x02, 0,0, 0,0, 0,0, 0,0, 0x00, 0x00, '\r', '\n' };



// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
  mpuInterrupt = true;
}



// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================


// =====================
// Serial Variables
// =====================
char computerChar;
char arduinoChar;

// =====================
// Light Variables
// =====================
int ledPins[] = {
  9, 10, 11};

// ==========================
// LightCycle Mode Variables
// ==========================
boolean freeFalling; // set to true if accelerometer is experiencing freefall; false if not
int throwStartTime; // time in milliseconds between the program start and the last time the accelerometer entered freefall
int throwEndTime; // time in milliseconds between the program start and the last time the accelerometer exited freefall
int lastThrowDuration = 100; // the duration in milliseconds that it was last in freefall (throwEndTime - throwStartTime). First throw 100 default arbitrary - fix?

// ========================
// Initial Config Variables
// ========================
// Distance in cm (mm?) from center of mass of the ball. Used for centripetal acceleration offset in detecting freefall.
int distFromCOM = 2; // TODO: Allow the user to measure and enter this somehow.

// ====================================
// Variables Relating to Changing Modes
// ====================================
// Escape char
int ESCAPE_CHAR = 27;

// Button Pin
int BUTTON_PIN = 15;

// Mode
int mode = 0;

// Number of modes in mode selection
int NUMBER_OF_MODES = 8;

// Amount of time to wait to advance from mode selection
int MODE_SELECTION_WAIT = 7000;

/*
Predefined colors
 */
int RED[] = {
  255, 0, 0};    
int GREEN[] = {
  0, 255, 0}; 
int BLUE[] = {
  0, 0, 255}; 
int YELLOW[] = {
  255, 255, 0}; 
int CYAN[] = {
  0, 255, 255}; 
int MAGENTA[] = {
  255, 0, 255};
int WHITE[] = {
  255, 255, 255}; 
int BLACK[] = {
  0, 0, 0}; 

int* currentColor = WHITE;

int* COLORS[] = {
  RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA    }; 

void setup() {
  // ====================
  // SERIAL COMMUNICATION
  // ====================
  Serial.begin(9600);
  Serial1.begin(9600);

  // ====================
  // HARDWARE SETUP
  // ====================
  
  /*
  Set up LEDs
   */
  for(int i = 0;i < 3;i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  /*
  Set up button
   */
  pinMode(BUTTON_PIN, INPUT);

  //initializes the random() function with analog noise from an unused pin
  randomSeed(analogRead(0)); //is this an analog input on the fio?

  // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
  TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif

  // initialize serial communication
  // (115200 chosen because it is required for Teapot Demo output, but it's
  // really up to you depending on your project)
  //while (!Serial); // wait for Leonardo enumeration, others continue immediately

  // NOTE: 8MHz or slower host processors, like the Teensy @ 3.3v or Ardunio
  // Pro Mini running at 3.3v, cannot handle this baud rate reliably due to
  // the baud timing being too misaligned with processor ticks. You must use
  // 38400 or slower in these cases, or use some kind of external separate
  // crystal solution for the UART timer.

  // initialize device
  Serial.println(F("Initializing I2C devices..."));
  mpu.initialize();

  // verify connection
  Serial.println(F("Testing device connections..."));
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

  //we didn't want to wait for ready...so we commented this out.  It initializes automatically now.
  // wait for ready
  //Serial.println(F("\nSend any character to begin DMP programming and demo: "));
  //while (Serial.available() && Serial.read()); // empty buffer
  //while (!Serial.available());                 // wait for data
  //while (Serial.available() && Serial.read()); // empty buffer again

  // load and configure the DMP
  Serial.println(F("Initializing DMP..."));
  devStatus = mpu.dmpInitialize();

  // supply your own gyro offsets here, scaled for min sensitivity
  // we should determine these numbers 
  mpu.setXGyroOffset(220);
  mpu.setYGyroOffset(76);
  mpu.setZGyroOffset(-85);
  mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

  // make sure it worked (returns 0 if so)
  if (devStatus == 0) {
    // turn on the DMP, now that it's ready
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);

    // enable Arduino interrupt detection
    Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
    attachInterrupt(0, dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();

    // set our DMP Ready flag so the main loop() function knows it's okay to use it
    Serial.println(F("DMP ready! Waiting for first interrupt..."));
    dmpReady = true;

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
  } 
  else {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }

  // configure LED for output
  pinMode(LED_PIN, OUTPUT);

  enterModeSelection();
}

double maxOverallAccel = 0;
double avgAccel = 0;
double accelValues = 0;

// =======================================================================================================================
//                             MAIN PROGRAM LOOP                    
// =======================================================================================================================

void loop() {
  // if programming failed, don't try to do anything
  if (!dmpReady) return;

  // wait for MPU interrupt or extra packet(s) available
  while (!mpuInterrupt && fifoCount < packetSize) {
    // other program behavior stuff here
    // .
    // .
    // .
    // if you are really paranoid you can frequently test in between other
    // stuff to see if mpuInterrupt is true, and if so, "break;" from the
    // while() loop to immediately process the MPU data
    // .
    // .
    // .
  }

  // reset interrupt flag and get INT_STATUS byte
  mpuInterrupt = false;
  mpuIntStatus = mpu.getIntStatus();

  // get current FIFO count
  fifoCount = mpu.getFIFOCount();

  // check for overflow (this should never happen unless our code is too inefficient)
  if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
    // reset so we can continue cleanly
    mpu.resetFIFO();
    Serial.println(F("FIFO overflow!"));

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
  } 
  else if (mpuIntStatus & 0x02) {
    // wait for correct available data length, should be a VERY short wait
    while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

    // read a packet from FIFO
    mpu.getFIFOBytes(fifoBuffer, packetSize);

    // track FIFO count here in case there is > 1 packet available
    // (this lets us immediately read more without waiting for an interrupt)
    fifoCount -= packetSize;

    // blink LED to indicate activity
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);

    //main output

    //THIS IS WHERE YOU SHOULD DO STUFF

    // Receives information from computer
    receiveCharFromComputer();

    // Receives information from arduino
    receiveCharFromArduino();

    checkForEscape();


    switch(mode) {
    case 0:
      Serial.println("Running red blue acceleration");
      fadeRedBlueAcceleration();
      Serial.println("End red blue acceleration");
      break;
    case 1:
      Serial.println("Running hot potato");
      hotPotato();
      Serial.println("End hot potato");
      break;
    case 2:
      lightCycle();
      break;
    case 3:
      Serial.println("Running temperature");
      while (true) { 
        double temp = ((double)mpu.getTemperature() + 12412.0) / 340.0;
        Serial.println(temp);
      }
      break;
    }           

  }
}


// =====================
// SERIAL HELPER METHODS
// =====================

/**
 * Receive a character from the computer.
 */
void receiveCharFromComputer() {
  if (Serial1.available()) {
    computerChar = (char) Serial1.read();
    Serial.print(computerChar);
  }
}

/**
 * Receive a character from the Arduino.
 */
void receiveCharFromArduino() {
  if (Serial.available()) {
    arduinoChar = (char) Serial.read();
    Serial1.print(arduinoChar);
  }
}

void checkForEscape() {
  if (computerChar == 'r') {
      computerChar = 'A';
      blinkLight(125);
      blinkLight(125);
      blinkLight(125);
      delay(250);
      enterModeSelection();
    }
}

// ====================
// MODE SELECTION
// ====================
/*
  Code to change the mode. The mode should change when the button is clicked.
 The light should also change to reflect that change.
 */
void enterModeSelection() {
  Serial.println("Button Stage");
  Serial1.print("Enter Mode: 0 - ");
  Serial1.println(NUMBER_OF_MODES); 


  // Convert NUMBER_OF_MODES to a string so that can access characters
  String numModesString = String(NUMBER_OF_MODES);

  boolean buttonClicked = false;
  int modeSelectionStartTime = millis();
  changeModeLight(mode); //lights up the leds

  while (true) {
    // Receives information from computer
    receiveCharFromComputer();

    // Receives information from arduino
    receiveCharFromArduino();

    if (computerChar >= '0' && computerChar <= numModesString[0]) {
      mode = computerChar - '0';
      changeModeLight(mode);
      computerChar = 'A';
      break;
    }

    if (!buttonClicked && digitalRead(BUTTON_PIN) == LOW) {
      buttonClicked = true;
      mode = (mode + 1) % NUMBER_OF_MODES;
      changeModeLight(mode);
      modeSelectionStartTime = millis();
      Serial.print("Button pressed: Mode ");
      Serial.println(mode); 
    }

    if (buttonClicked && digitalRead(BUTTON_PIN) == HIGH) {
      buttonClicked = false;
      Serial.println("Button released");
    }

    int timeWaited = millis() - modeSelectionStartTime;
    if (timeWaited > MODE_SELECTION_WAIT) break;
  }

  blinkLight(125);
  blinkLight(125);
  blinkLight(125);
  delay(250);

  Serial.print("End Button Stage, Mode Selected: ");
  Serial.println(mode);
  
  Serial1.print("Mode Selected: ");
  Serial1.println(mode);
}

// ====================
// LIGHT HELPER METHODS
// ====================

void blinkLight(int pause) {
  lightOff();
  delay(pause);
  lightOn();
  delay(pause);
  lightOff();
}

void lightOff() {
  setColor(ledPins, BLACK);
}

void lightOn() {
  setColor(ledPins, currentColor);
}

// Returns the pythagorean sum of the x-, y-, and z-rotation values
// Comment: How accurate is this? Can rotation speed components be added like vectors?
int getRotationSpeed() {
  int xRot = abs(mpu.getRotationX());
  int yRot = abs(mpu.getRotationY());
  int zRot = abs(mpu.getRotationZ());

  // If this goes wacko, it's int overflow - change the ints to longs and cast it back at the end.
  return sqrt(pow(xRot,2) + pow(yRot,2) + pow(zRot,2));
}

// Normalizes getRotationSpeed() to never be more than 255
int getRotationSpeed255() { 
  int maxRot = getRotationSpeed();
  int rotVal = maxRot * (255/8000.0);
  rotVal = (rotVal > 255) ? 255 : rotVal; //normalize it to never be more than 255
  return rotVal;
}
// Fade from red to blue based on rotation speed
void fadeRedBlueRotation(){
  int rotVal = getRotationSpeed255();
  draw(rotVal, 0, 255 - rotVal); // R-B
  //fadeRedBlueGreen(rotVal); // R-B-G 
}

// Fade from red to blue to green based on rotational speed
// Doesn't work all that well, the colors don't fade very evenly so it flickers a lot
void fadeRBGRotation(){
  int rotVal = getRotationSpeed255();
  fadeRedBlueGreen(rotVal); // R-B-G 
}

//fade from red to blue based on z component of gravity
void fadeRedBlueGravity(){
  //read the z component of the gravity, and adjust the LED accordingly
  mpu.dmpGetQuaternion(&q, fifoBuffer);
  mpu.dmpGetGravity(&gravity, &q);
  Serial.print(gravity.x);
  Serial.print("\t");
  Serial.print(gravity.y);
  Serial.print("\t");
  Serial.println(gravity.z);
  int valueZ = 128 + gravity.z * 127; //0-255
  draw(255 - valueZ, 0, valueZ); //fade red to blue */
  //Serial.println(valueZ);

}

//fades from red to blue based on acceleration (doesn't really work yet)
void fadeRedBlueAcceleration(){
  mpu.dmpGetQuaternion(&q, fifoBuffer);
  mpu.dmpGetAccel(&aa, fifoBuffer);
  mpu.dmpGetGravity(&gravity, &q);
  mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
  int xAccel = aaReal.x;
  int yAccel = aaReal.y;
  int zAccel = aaReal.z;
  int maxAccelXY = (xAccel > yAccel) ? xAccel : yAccel;
  int maxAccel = (maxAccelXY > zAccel) ? maxAccelXY : zAccel;


  // Calculating stats (just for testing, don't worry about it)
  /*
  maxOverallAccel =  (maxOverallAccel > maxAccel) ? maxOverallAccel : maxAccel;
   accelValues++;
   if (accelValues == 1){
   avgAccel = maxAccel;
   } else{
   avgAccel = avgAccel - avgAccel/accelValues + maxAccel/accelValues;
   }*/

  Serial.print("\t");
  Serial.print(xAccel); 
  Serial.print("\t");
  Serial.print(yAccel); 
  Serial.print("\t");
  Serial.print(zAccel); 
  Serial.print("\t");
  Serial.print(maxAccel);


  // Red to Blue based on acceleration
  int accelVal = (maxAccel-3500) * ((double)255/4000.0);
  if (accelVal < 0){
    accelVal = 0;
  }
  Serial.print("\t");
  Serial.println(accelVal);
  accelVal = (accelVal > 255) ? 255 : accelVal;
  draw(accelVal, 0, 255 - accelVal); // red to blue
} 

void draw(int r, int g, int b) {
  //sets LED color based on RGB values
  analogWrite(color('r'), 255 - r);
  analogWrite(color('g'), 255 - g);
  analogWrite(color('b'), 255 - b);
}

int color(char color) {
  //returns pins for each color according to ledPins (defined up top)
  switch(color) {
  case 'r':
    return ledPins[0];
    break;

  case 'g':
    return ledPins[1];
    break;

  case 'b':
    return ledPins[2];
    break;
  }  
}

void fadeRedBlueGreen(int val){
  int r = (val*2 - 255 > 0) ? (val*2 - 255) : 0;
  int b = (128 - val) *2;
  int g = ((((255 - val) * 2) - 255) > 0) ? ((255 - val) * 2 - 255) : 0;
  draw(r, g, b); 
}

/*
 * Precondition: 0 <= val, rStopVal, bStartVal <= 1
 * Fades from red to blue in a nice, nonlinear way.
 * By default (if only passed one argument, val), it's a  changing shade of purple the whole time.
 * To have red be completely turned off before the throw's end (so it's just blue of increasing brightness), input rStopVal such that 0.5 <= rStopVal < 1.
 * Likewise, to have blue only start coming on partway in, input bStartVal such that 0.0 < bStartVal <= 0.5.
 */
void fadeRedBlueParabolically(float val, float rStopVal = 1.0, float bStartVal = 0.0) {
  // Following coordinates in comments are in (time, brightness value) format
  int r = 255 * (1 - pow(val/rStopVal, 2)); // Given by parabola with vertex at (0, 255) and x-intercept (rStopVal, 0)
  int b = 255 * (1 - pow((val-1)/bStartVal, 2)); // Given by parabola with x-intercept at (bStartVal, 0) and vertex at (1, 255);
  // We're guaranteed r, b <= 1 because 0<=val<=1; however, we still need to make sure r, b >= 0.
  r = (r < 0) ? 0 : r;
  b = (b < 0) ? 0 : b;

  draw(r, 0, b); 
}

//=====Hot Potato Feature=======

void hotPotato() {

  int t_on = 100; //time (in ms) that the led should be on (per blink)
  int t_off0 = 1000; //time (in ms) that the led should be off at the loop's start
  int t_off; //time that the led should be off (per blink). this will change over the course of the loop

  int rON[] = {
    255,0,0          }; //lights red LED
  int bON[] = {
    0,0,255          }; //lights blue LED
  int OFF[] = {
    0,0,0          };   

  int duration = random(10000,30000); //length of one hot-potato game in ms. chosen randomly to be between 10 and 30 seconds
  int t_lastSwitch = 0;
  int t = 0;

  Serial.println(duration);

  boolean isOn = true;
  setColor(ledPins, bON);

  while (t < duration) {
    
    // Receives information from computer
    receiveCharFromComputer();

    // Receives information from arduino
    receiveCharFromArduino();
    
    checkForEscape();

    //these if statements blink the blue led
    if (isOn){
      if (t - t_lastSwitch > t_on) { //if the led's on, check if it's time to turn it off
        isOn = false;
        t_lastSwitch = t;
      }
    } 
    else {//if the led's off, check if it's time to turn it on
      t_off = map(t, 0, duration, t_off0, 0); //scales the off time down linearly according to how close the game is to ending 
      //(i.e. according to how t compares with duration)
      if (t - t_lastSwitch > t_off) {  
        isOn = true;
        t_lastSwitch = t;
      }
    }

    if (isOn) setColor(ledPins, bON); 
    else setColor(ledPins, OFF);

    t += 10;
    delay(10);
  }

  while (isFreeFalling()){
    delay(500); //don't end the game while the ball is in mid-throw
    Serial.println("Free Fall!");
  }

  setColor(ledPins, rON); //game is over, turn the led red


  while (digitalRead(BUTTON_PIN) == LOW) {
    //waits for the user to press the reset button to reset the game
  }


}

//for common anode led
void setColor(int* led, int* color){
  for(int i = 0; i < 3; i++){
    analogWrite(led[i], 255-color[i]); //note: a low output turns the led on
  }
}

// one possible test to see whether the ball is in free fall. Assumes the MPU 6050 has 
// already been properly initialized and that "VectorInt16 aa;" has been declared
boolean isFreeFalling() {
  mpu.dmpGetQuaternion(&q, fifoBuffer);
  mpu.dmpGetAccel(&aa, fifoBuffer);
  mpu.dmpGetGravity(&gravity, &q);
  mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
  int xAcc = aa.x;
  int yAcc = aa.y;
  int zAcc = aa.z;

  int a = sqrt(pow(xAcc,2) + pow(yAcc,2) + pow(zAcc,2)); //magnitude of the acceleration
  // NOTE: pow(_Acc,2) may cause int overflow! If weird numbers start coming out, try the following:
  /*
  long xAcc = (long) aa.x;
   long yAcc = (long) aa.y;
   long zAcc = (long) aa.z;
   int a = (int) sqrt(pow(xAcc,2) + pow(yAcc,2) + pow(zAcc,2));
   */

  return (a < 1000) ? true : false; //the 1000 here is essentially arbitrary
  /*
  // If the ball is spinning and the MPU is not at the ball's center of mass (COM), the MPU will experience (and report) an extra
   // centripetal acceleration. This needs to be removed to get the proper acceleration of the ball's COM for proper freefall detection.
   int rotVal = getRotationSpeed() / 500; //this 500 is arbitrary but important to fine-tune
   int centripetalOffset = pow(rotVal, 2)*distFromCOM;
   int aCOM = a - centripetalOffset;
   return (aCOM < 1000) ? true : false;
   */
}

void changeModeLight(int mode) {
  int col = mode%6; //loop back around if mode > 5

  Serial.println(col);
  setColor(ledPins, COLORS[col]);
}


//=========Light Cycle Feature==========
/*
 * Times the duration of each throw, and fades from red to blue linearly or parabolically using the duration of the previous throw.
 *
 * Note: this function is designed delay free to avoid FIFO overflow. So, the program will be quickly alternating between the main loop and this function.
 * Pros of this approach: No FIFO overflow -> better MPU sensing
 * Cons of this approach: Requires more global variables
 *
 * TODO: maybe have it accept a light output function as a parameter?
 */
void lightCycle() {

  // Detecting the start of a throw
  if (isFreeFalling() && !freeFalling) { 
    freeFalling = true;
    throwStartTime = millis();  // millis() is the current time since program start in milliseconds
    Serial.println("Free fall started!"); // for debugging purposes
    Serial1.println("Free fall started!"); // for wireless debugging purposes
  }

  // Updating the lights mid-throw
  if (isFreeFalling() && freeFalling) {
    int airTime = millis() - throwStartTime; // Time in milliseconds since leaving the hand
    float cycleProgress = ((double) airTime) / lastThrowDuration; // "Progress" through the cycle of the throw (described better below)
    // TODO: clean up the casting?
    cycleProgress = (cycleProgress > 1) ? 1 : cycleProgress;
    // cycleProgress = 0 when it leaves the hand, then increases linearly until "lastThrowDuration" milliseconds after leaving the hand, where it stays at 1
    // Note: 0 <= cycleProgress <= 1

    // === Linear Fading ====
    int cycleProgress255 = (int) (cycleProgress * 255);
    int red = 255 - cycleProgress255; // Starts at 255, linearly declines to 0
    int blue = cycleProgress255; // Starts at 0, linearly increases to 255
    draw(red, 0, blue);

    // === Parabolic Fading ====
    // fadeRedBlueParabolically(cycleProgress, 0.6, 0.4);
  }  

  // Detecting the end of a throw   
  // If it is no longer in free fall and we haven't set freeFalling back to false yet, set it to false, and set throwEndTime and lastThrowDuration.
  // Potential issue: is the minimum accel threshold for !isFreeFalling() too low? 
  // Do we need a isStronglyAccelerating() function with a higher minimum threshold instead of !isFreeFalling()?
  if (freeFalling && !isFreeFalling()) {
    freeFalling = false;
    throwEndTime = millis();
    lastThrowDuration = throwEndTime - throwStartTime;
    Serial.println("Free fall stopped. Duration:"); // for debugging purposes
    Serial.println(lastThrowDuration); // for debugging purposes
    Serial1.println("Free fall stopped. Duration:"); // for wireless debugging purposes
    Serial1.println(lastThrowDuration); // for wireless debugging purposes
  }
}



