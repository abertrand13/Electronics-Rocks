
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
// Number of loops
// =====================
int numLoops = 0;

// =====================
// Serial Variables
// =====================
String computerString;
String arduinoString;

// =====================
// Light Variables
// =====================
int ledPins[] = {
  9, 10, 11};

int redPin = 11;
int greenPin = 10;
int bluePin = 9;

// ==========================
// LightCycle Mode Variables
// ==========================
boolean freeFalling; // set to true if accelerometer is experiencing freefall; false if not
unsigned long throwStartTime; // time in milliseconds between the program start and the last time the accelerometer entered freefall
unsigned long throwEndTime; // time in milliseconds between the program start and the last time the accelerometer exited freefall
int lastThrowDuration = 100; // the duration in milliseconds that it was last in freefall (throwEndTime - throwStartTime). First throw 100 default arbitrary - fix?

const int historyLength = 10; // Keep it even, to be safe. The greater this is, the less sensitive LightCycle is to spiking acceleration (which helps muffle noise)
int accelHistory[historyLength];
int recentAvgAccel = 0;
int olderAvgAccel = 0;
int historyIndex = 0;

// ========================
// Initial Config Variables
// ========================
// Distance in cm (mm?) from center of mass of the ball. Used for centripetal acceleration offset in detecting freefall.
int distFromCOM = 2; // TODO: Allow the user to measure and enter this somehow.

boolean usingWiredSerial = true;
boolean usingWirelessSerial = true;

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
int NUMBER_OF_MODES = 10;

// Amount of time to wait to advance from mode selection
int MODE_SELECTION_WAIT = 7000;

// ====================================
// Color Variables
// ====================================
// Predefined Colors
// TODO: Convert to bytes to save memory?
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

byte PINK[] = {
  158, 4, 79}; 
byte INDIGO[] = {
  4, 0, 19}; 
byte ORANGE[] = {
  83, 4, 0}; 
byte BLUE2[] = {
  0, 0, 225}; 

// Current color
int* currentColor = WHITE;

// Array of colors
int* COLORS[] = {
  RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA}; 

void setup() {
  // ====================
  // SERIAL COMMUNICATION
  // ====================
  if (usingWiredSerial)
    Serial.begin(9600);
  if (usingWirelessSerial)
    Serial1.begin(9600);

  // Useful for debugging. Remove for final product
  if (usingWiredSerial)
    while(!Serial);

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
// =======================================================================================================================
// ==========================                    MAIN PROGRAM LOOP                    ====================================
// =======================================================================================================================
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

    //THIS IS WHERE YOU SHOULD DO STUFF
    // Receives information from computer
    receiveStringFromComputer();

    // Receives information from arduino
    receiveCharFromArduino();

    if (checkForEscape()) {
      enterModeSelection();
    }

    // ============================
    //  MODE SWITCH STATEMENT
    // ============================
    switch(mode) {
    case 0:
      {
        fadeRedBlueAcceleration();
      }
      break;
    case 1:
      {
        fadeRedBlueRotation();
      }
      break;
    case 2:
      {
        hotPotato();
      }
      break;
    case 3:
      {
        redToBlueFade(500, redPin, bluePin);
      }
      break;
    case 4:
      {
        lightCycle();
        updateAccelHistory();
        //displayIfInFreeFall();
      }
      break;
    case 5:
      {
        userLightInput();
      }
      break;
    case 6:
      {
        showSensorTemperature();
      }
      break; 
    case 7:
      {
        showWebTemperature();
      }
      break;
    case 8: 
      {
        accelerationFlashing();
      }
      break;
    case 9: 
      {
        gravityGlow();
      }
      break;
    }
  }
}

// =======================================================================================================================
// =======================================================================================================================
// ================================                    FEATURES                    =======================================
// =======================================================================================================================
// =======================================================================================================================


// ==============================================
// FADE RED TO BLUE BASED ON ACCELERATION FEATURE
// ==============================================
/*
 * fades from red to blue based on acceleration (doesn't really work yet)
 */
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

// ==============================================
// ROTATION FADE FEATURE
// ==============================================

// Fade from red to blue based on rotation speed
void fadeRedBlueRotation(){
  int rotVal = getRotationSpeed255();
  draw(rotVal, 0, 255 - rotVal); // R-B
  //fadeRedBlueGreen(rotVal); // R-B-G 
}

// ==============================================
// HOT POTATO FEATURE
// ==============================================
void hotPotato() {

  int t_on = 100; //time (in ms) that the led should be on (per blink)
  int t_off0 = 1000; //time (in ms) that the led should be off at the loop's start
  int t_off; //time that the led should be off (per blink). this will change over the course of the loop

  int rON[] = {
    255,0,0                                        }; //lights red LED
  int bON[] = {
    0,0,255                                        }; //lights blue LED
  int OFF[] = {
    0,0,0                                        };   

  int duration = random(10000,30000); //length of one hot-potato game in ms. chosen randomly to be between 10 and 30 seconds
  int t_lastSwitch = 0;
  int t = 0;

  Serial.println(duration);

  boolean isOn = true;
  setColor(ledPins, bON);

  while (t < duration) {

    // Receives information from computer
    receiveStringFromComputer();

    // Receives information from arduino
    receiveCharFromArduino();

    if (checkForEscape()) break;

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

  while (true) {//wait for the user to reset the game
    // Receives information from computer
    receiveStringFromComputer();
    // Receives information from arduino
    receiveCharFromArduino();
    if (checkForEscape()) break;

    if (digitalRead(BUTTON_PIN) == LOW) break;
  }
}


// ===========================================
// RED TO BLUE FADE FEATURE
// ===========================================
//Fades red to blue wit. Might need to change color more often.
// seems to work, but not tested with avg time function and real free fall
// note: my led is lit up at 0 and off at 255. If this is also true
// for led in juggling ball, must switch red and blue variables.
// Function needs pin number of red, blue, and time (in ms) fall expected to take.
void redToBlueFade(int time,int redPin,int bluePin) {
  int red; //will give how much red output from scale of 0 to 255
  int blue;

  unsigned long startTime=millis() % 1000000;
  unsigned long landTime=time+millis(); //time expected to land

  while (millis() < landTime) { //before it lands, change color often
    blue = map(millis(),startTime,landTime,0,255);
    blue= constrain(blue,0,255); // just in case something goes wrong
    red = map(millis(),startTime,landTime,255,0);
    red = constrain(red,0,255);


    analogWrite(redPin,red);     //outputs color
    analogWrite(bluePin,blue);

    delay(10); //wait before changing color again to limit crazy flow of data
  }
}


// ==============================================
// LIGHT CYCLE FEATURE (by Matt!)
// ==============================================
/*
 * Test feature to delete: displayIfInFreeFall()
 *
 */
void displayIfInFreeFall() {
  //Serial.println(getLinearAccelerationMagnitude());
  if (accelIsPlunging(0.33)){
    //Serial.print("\t\t");
    draw(255,255,0); 
    Serial.println("Plunge!!!!!!!!!!!!!!!!");
  }
  else if (accelIsSpiking(3.0)){
    draw (0,255,255);
    Serial.println("Spike!!");
  }
  else {
    draw (255,255,255);
  }
  //Serial.println(getLinearAccelerationMagnitude());
}

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
  if (accelIsPlunging(0.33) && !freeFalling) { 
    freeFalling = true;
    throwStartTime = millis() % 100000;  // millis() is the current time since program start in milliseconds. Taken mod 100,000 to prevent [unlikely] overflow.
    Serial.println("Free fall started!"); // for debugging purposes
    Serial1.println("Free fall started!"); // for wireless debugging purposes
  }

  // Updating the lights mid-throw
  if (freeFalling) {
    int airTime = (int)(millis() - throwStartTime); // Time in milliseconds since leaving the hand
    float cycleProgress = ((double) airTime) / lastThrowDuration; // "Progress" through the cycle of the throw (described better below)
    // TODO: clean up the casting?
    cycleProgress = (cycleProgress > 1) ? 1 : cycleProgress;
    // cycleProgress = 0 when it leaves the hand, then increases linearly until "lastThrowDuration" milliseconds after leaving the hand, where it stays at 1
    // Note: 0 <= cycleProgress <= 1

    // === Linear Fading ====
    int cycleProgress255 = (int) (cycleProgress * 255);
    int red = 255 - cycleProgress255; // Starts at 255, linearly declines to 0
    int blue = cycleProgress255; // Starts at 0, linearly increases to 255
    draw(255-red, 255, 255-blue);

    // === Parabolic Fading ====
    // fadeRedBlueParabolically(cycleProgress, 0.6, 0.4);
  }  

  // Detecting the end of a throw   
  // If it is no longer in free fall and we haven't set freeFalling back to false yet, set it to false, and set throwEndTime and lastThrowDuration.
  // Potential issue: is the minimum accel threshold for !isFreeFalling() too low? 
  // Do we need a isStronglyAccelerating() function with a higher minimum threshold instead of !isFreeFalling()?
  if (freeFalling && accelIsSpiking(3.0)) {
    freeFalling = false;
    throwEndTime = millis() % 100000;
    int duration = throwEndTime - throwStartTime;
    if (duration > 10)
      lastThrowDuration = (duration > 32000) ? 32000 : duration;
    Serial.println("Free fall stopped. Duration:"); // for debugging purposes
    Serial.println(lastThrowDuration); // for debugging purposes
    Serial1.println("Free fall stopped. Duration:"); // for wireless debugging purposes
    Serial1.println(lastThrowDuration); // for wireless debugging purposes
  }
}

// ==============================================
// SENSOR TEMPERATURE FEATURE
// ==============================================
/*
 * Should show temperature from the sensor, and then map
 * it to an int between 0 and 255 so that it is visible.
 * Currently buggy. MPU doesn't seem to work properly.
 */
void showSensorTemperature() {
  double sensorTemp = ((double)mpu.getTemperature() + 12412.0) / 340.0;
  int sensorTempInt = (int)sensorTemp;
  draw(sensorTempInt, 0, 255 - sensorTempInt);
}

// Juan's version - TODO: reconcile/combine with the above and add to mode choices
void setColorOnTemperature(int temp){
  //sets the color based on temperature

  if (temp >= 40) { //when it is hot
    fadeToColor(ledPins, ORANGE, INDIGO, 10);
    fadeToColor(ledPins, INDIGO, ORANGE, 10);
  } 
  else{ //when it is cold
    fadeToColor(ledPins, BLUE2, ORANGE, 10);
    fadeToColor(ledPins, ORANGE, BLUE2, 10); 
  }
}

// ==============================================
// WEB TEMPERATURE FEATURE
// ==============================================
void showWebTemperature() {
  numLoops++;
  if (numLoops % 100 == 0) {
    Serial1.println("Getting Temperature");
    receiveStringFromComputer();
    int webTemp = computerString.toInt();
    draw(webTemp, 0, 255 - webTemp);
  }
}

// ==============================================
// ACCELERATION FLASHING FEATURE by Juan! 
// ==============================================
/*
 * Flashes red, blue, green very quickly if it's accelerating a lot, less quickly otherwise.
 * Currently causes FIFO overflow because of the delays.
 */
void accelerationFlashing() {
  int accel = getLinearAccelerationMagnitude();
  int timeDelay = 160; //change as you like!
  if (accel > 5000)
    rapidFlashing(timeDelay/5);
  else 
    rapidFlashing(timeDelay);
}

void rapidFlashing(int time) {
  //LED flashes from red to blue to green at a rate based on maximum acceleration
  setColor(ledPins, RED);
  delay(time);
  setColor(ledPins, BLUE);
  delay(time);
  setColor(ledPins, GREEN);
  delay(time);
}

// ================
// GRAVITY GLOW
// ================
/*
 * Changes color according to the current orientation of the ball.
 */
void gravityGlow() {
  mpu.dmpGetQuaternion(&q, fifoBuffer);
  mpu.dmpGetGravity(&gravity, &q);
  //draw((int) 127*(1+gravity.x),(int) 127*(1+gravity.y),(int) 127*(1+gravity.z));
  draw((int) 255*(1-abs(gravity.x)),(int) 255*(1-abs(gravity.y)),(int) 255*(1-abs(gravity.z))); // Another option
}


// =======================================================================================================================
// =======================================================================================================================
// =============================                    HELPER METHODS                    ====================================
// =======================================================================================================================
// =======================================================================================================================

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
  unsigned long modeSelectionStartTime = millis() % 100000;
  changeModeLight(mode); //lights up the leds

  while (true) {
    // Receives information from computer
    receiveStringFromComputer();
    // Receives information from arduino
    receiveCharFromArduino();

    if (computerString.length() >= 1 && computerString[0] >= '0' && computerString[0] <= '9' && computerString.toInt() <= NUMBER_OF_MODES) {
      mode = computerString.toInt();
      Serial.print("Key pressed: Mode ");
      Serial.println(mode); 
      changeModeLight(mode);
      break;
    }

    if (!buttonClicked && digitalRead(BUTTON_PIN) == LOW) {
      buttonClicked = true;
      mode = (mode + 1) % NUMBER_OF_MODES;
      changeModeLight(mode);
      modeSelectionStartTime = millis() % 100000;
      Serial.print("Button pressed: Mode ");
      Serial.println(mode); 
    }

    if (buttonClicked && digitalRead(BUTTON_PIN) == HIGH) {
      buttonClicked = false;
      Serial.println("Button released");
    }

    int timeWaited = (int)(millis() - modeSelectionStartTime);
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

// =======================================
// NETWORKING HELPER METHODS
// =======================================

/**
 * Receive a character from the computer.
 */
void receiveStringFromComputer() {
  if (Serial1.available()) {
    computerString = "";
    while (true) {
      if (Serial1.available()) {
        char computerChar = (char) Serial1.read();
        if (computerChar == '\n') {
          Serial.println(computerString);
          break;
        }
        computerString += computerChar;
      }
    }
  }
}

/**
 * Receive a character from the Arduino.
 */
void receiveCharFromArduino() {
  if (Serial.available()) {
    char arduinoChar = (char) Serial.read();
    arduinoString += arduinoChar;
    Serial1.print(arduinoChar);
  }
}

/**
 * Checks for the escape key from the computer.
 */
boolean checkForEscape() {
  if (computerString.length() == 1 && computerString[0] == 'r') {
    blinkLight(125);
    blinkLight(125);
    blinkLight(125);
    delay(250);
    return true;
  }
  return false;
}

/**
 * Gets RGB values from the user via the Serial and alters
 * the light accordingly.
 */
void userLightInput() {
  // Receives information from computer
  receiveStringFromComputer();

  // Receives information from arduino
  receiveCharFromArduino();

  if (computerString.length() >= 7 && computerString[0] == 'C') {
    int red;
    int green;
    int blue;
    int counter = 0;
    String current = "";
    for (int i = 8; i < computerString.length(); i++) {
      if (computerString[i] != ' ') {
        current += computerString[i];
      } 
      else {
        switch(counter) {
        case 0:
          red = current.toInt();
          counter++;
          current = "";
          break;
        case 1:
          green = current.toInt();
          counter++;
          current = "";
          break;
        case 2:
          blue = current.toInt();
          current = "";
          break;
        }
      }
    }

    draw(red, green, blue);
  }
}


// =======================================
// LIGHT HELPER METHODS
// =======================================

/* 
 * Changes the light depending on the mode.
 */
void changeModeLight(int mode) {
  int col = mode % 6; //loop back around if mode > 5

  currentColor = COLORS[col];
  setColor(ledPins, currentColor);
}

/**
 * Blinks the light between on and off.
 */
void blinkLight(int pause) {
  lightOff();
  delay(pause);
  lightOn();
  delay(pause);
  lightOff();
}

/**
 * Turns the light off.
 */
void lightOff() {
  setColor(ledPins, BLACK);
}

/**
 * Turns the light on.
 */
void lightOn() {
  setColor(ledPins, currentColor);
}

/**
 * Sets LED color based on RGB values.
 */
void draw(int r, int g, int b) {
  analogWrite(color('r'), 255 - r);
  analogWrite(color('g'), 255 - g);
  analogWrite(color('b'), 255 - b);
}

/**
 * returns pins for each color according to ledPins (defined up top)
 */
int color(char color) {
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

/**
 * Sets the color using an int array.
 */
void setColor(int* led, int* color){
  for(int i = 0; i < 3; i++){
    analogWrite(led[i], 255-color[i]); //note: a low output turns the led on
  }
}

void setColor(int* led, byte* color){
  for(int i = 0; i< 3; i++){
    analogWrite(led[i], 255-color[i]);  
  }
}

/**
 * Fades from startColor to endColor with speed fadeSpeed.
 * Uses bytes for colors instead of ints to save memory.
 */
void fadeToColor(int* led, byte* startColor, byte* endColor, int fadeSpeed){
  int changeRed = endColor[0] - startColor[0];                            //the difference in the two colors for the red channel
  int changeGreen = endColor[1] - startColor[1];                          //the difference in the two colors for the green channel 
  int changeBlue = endColor[2] - startColor[2];                           //the difference in the two colors for the blue channel
  int steps = max(abs(changeRed),max(abs(changeGreen), abs(changeBlue))); //make the number of change steps the maximum channel change

    for(int i = 0 ; i < steps; i++){                                        //iterate for the channel with the maximum change
    byte newRed = startColor[0] + (i * changeRed / steps);                 //the newRed intensity dependant on the start intensity and the change determined above
    byte newGreen = startColor[1] + (i * changeGreen / steps);             //the newGreen intensity
    byte newBlue = startColor[2] + (i * changeBlue / steps);               //the newBlue intensity
    byte newColor[] = {
      newRed, newGreen, newBlue    };                         //Define an RGB color array for the new color
    setColor(ledPins, newColor);                                               //Set the LED to the calculated value
    delay(fadeSpeed);                                                      //Delay fadeSpeed milliseconds before going on to the next color
  }
  setColor(led, endColor);                                                //The LED should be at the endColor but set to endColor to avoid rounding errors
}

// A version of fadeToColor that takes predefined colors (neccesary to allow const int pre-defined colors */
void fadetoColor(int* led, const byte* startColor, const byte* endColor, int fadeSpeed){
  byte tempColor1[] = {
    startColor[0], startColor[1], startColor[2]  };
  byte tempColor2[] = {
    endColor[0], endColor[1], endColor[2]  };
  fadeToColor(ledPins, tempColor1, tempColor2, fadeSpeed);
}

// =======================================
// SENSOR INPUT HELPER METHODS
// =======================================

/*
 * Returns the pythagorean sum of the x-, y-, and z-rotation values
 * Comment: How accurate is this? Can rotation speed components be added like vectors?
 */
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

/*
 * Returns the magnitude of linear acceleration with gravity removed.
 */
int getLinearAccelerationMagnitude() {
  mpu.dmpGetQuaternion(&q, fifoBuffer); // Required for removing effects of gravity
  mpu.dmpGetAccel(&aa, fifoBuffer); // Sets aa to the acceleration registered
  mpu.dmpGetGravity(&gravity, &q); // Sets gravity to a unit vector in the direction of gravity (if not in free fall)
  mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity); // Sets aaReal to the linear acceleration with gravity removed.
  //The below are longs instead of ints because we'll need them to be longs when we square them (they go over int's max value)
  long xAcc = aa.x; 
  long yAcc = aa.y;
  long zAcc = aa.z;
  return sqrt(pow(xAcc, 2) + pow(yAcc, 2) + pow(zAcc, 2));
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


// =======================
// IS FREE FALLING
// =======================
// one possible test to see whether the ball is in free fall. Assumes the MPU 6050 has 
// already been properly initialized and that "VectorInt16 aa;" has been declared
// unfortunately, not very accurate yet...
// i2CdevLib's getIntFreefallStatus does not always work either.
boolean isFreeFalling() {
  int a = getLinearAccelerationMagnitude(); //magnitude of the acceleration
  // NOTE: pow(_Acc,2) may cause int overflow! If weird numbers start coming out, try the following:
  /*
  long xAcc = (long) aa.x;
   long yAcc = (long) aa.y;
   long zAcc = (long) aa.z;
   int a = (int) sqrt(pow(xAcc,2) + pow(yAcc,2) + pow(zAcc,2));
   */

  //return (a < 2000) ? true : false; //the 1000 here is essentially arbitrary

  // If the ball is spinning and the MPU is not at the ball's center of mass (COM), the MPU will experience (and report) an extra
  // centripetal acceleration. This needs to be removed to get the proper acceleration of the ball's COM for proper freefall detection.
  int rotVal = getRotationSpeed() / 500; //this 500 is arbitrary but important to fine-tune
  int centripetalOffset = pow(rotVal, 2)*distFromCOM;
  //Serial.println(centripetalOffset); // usually tiny; can get up to 2500 if turning fast 
  int aCOM = a - centripetalOffset;
  return (aCOM < 3000) ? true : false;

}

// ====================================
// SHARP ACCELERATION CHANGE DETECTION
// ====================================

/*
 * The arduino stores its magnitude of acceleration over the past historyLength readings in the global array accelHistory[]. 
 * This function updates it.
 * TODO: Re-implement as a pair of queues to clean up all the % syntax? (functionally identical)
 */
void updateAccelHistory() {
  int accelToInsert = getLinearAccelerationMagnitude(); // current acceleration
  int accelToRemove = accelHistory[historyIndex]; // acceleration we're replacing in the history array (historyIndex is the index of the spot in the array to update)
  int midIndex = (historyIndex+historyLength/2) % historyLength;
  int midAccel = accelHistory[midIndex]; // acceleration that was part of recentAvgAccel and is now becoming part of olderAvgAccel
  recentAvgAccel = recentAvgAccel + (accelToInsert - midAccel)/(historyLength/2); // update recentAvgAccel
  olderAvgAccel = olderAvgAccel + (midAccel - accelToRemove)/(historyLength/2); 
  accelHistory[historyIndex] = accelToInsert;
  historyIndex = (historyIndex + 1) % historyLength; // % operation used to make it cycle from 0 to historyLength-1

  //   Serial.print("recentAvgAccel:");
  //   Serial.print("\t");
  //   Serial.print(recentAvgAccel);
  //   Serial.print("\t");
  //   Serial.print("olderAvgAccel:");
  //   Serial.print("\t");
  //   Serial.print(olderAvgAccel);
  //   Serial.print("\t");
  //   Serial.print("currentAccel:");
  //   Serial.print("\t");
  //   Serial.print(accelToInsert);
  //   Serial.print("\t");
  //   Serial.print("midAccel:");
  //   Serial.print("\t");
  //   Serial.print(midAccel);
  //   Serial.print("\t");
  //   Serial.print("midIndex:");
  //   Serial.print("\t");
  //   Serial.print(midIndex);
  //   Serial.print("\t");
  //   Serial.print("accelToRemove:");
  //   Serial.print("\t");
  //   Serial.println(accelToRemove);
}

/*
 * Returns whether the average of magnitudes of the past 5 acceleration readings is at least thresholdPercentage times the previous 5.
 * E.g. if thresholdPercentage = 2.0, it will return true if and only if the past 5 readings have on average at least 2x the accel magnitude as the 5 before that.
 * Uses averages from history to muffle noise reading noise.
 */
boolean accelIsSpiking(float thresholdPercentage) {
  return (recentAvgAccel >= olderAvgAccel*thresholdPercentage);
}

/*
 * Returns whether the average of magnitudes of the past 5 acceleration readings is less than thresholdPercentage times the previous 5.
 */
boolean accelIsPlunging(float thresholdPercentage) {
  return (recentAvgAccel < olderAvgAccel*thresholdPercentage);
}


