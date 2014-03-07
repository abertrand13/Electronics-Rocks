// Timing Throws
// 2/27/2014 by Matt Mistele <mmistele@stanford.edu>


// Uses code from I2C device class (I2Cdev) demonstration Arduino sketch for MPU6050 class 
// using DMP (MotionApps v2.0) -  6/21/2012 by Jeff Rowberg <jeff@rowberg.net>

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


#define LED_PIN 1 // (Arduino is 13, Teensy is 11, Teensy++ is 6)
bool blinkState = false;

// throw timing variables
boolean freeFalling; // set true if accelerometer is experiencing freefall; false if not
int throwStartTime; // time in milliseconds between the program start and the last time the accelerometer entered freefall
int throwEndTime; // time in milliseconds between the program start and the last time the accelerometer exited freefall
int lastThrowDuration; // the duration in milliseconds that it was last in freefall (throwEndTime - throwStartTime)

// wireless variables
char computerChar;
char arduinoChar; 

int RXLED = 17;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

VectorInt16 aa;         // [x, y, z]            accel sensor measurements

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

void setup() {
    // Initializing our variables
    freeFalling = false;
    throwStartTime = 0;
    throwEndTime = 0;
    lastThrowDuration = 0;  
  
    pinMode(RXLED, OUTPUT);
    
  // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif
    
    // initialize serial communication
    // Serial.begin(115200); // begin regular serial at maximum baud
    Serial.begin(9600); // begin regular serial at a good baud for transmitting wirelessly (?)
    Serial1.begin(9600); // begin wireless serial (at the same baud)
    
    while (!Serial); // wait for Leonardo enumeration, others continue immediately

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

    // wait for ready
    Serial.println(F("\nSend any character to begin DMP programming and demo: "));
    while (Serial.available() && Serial.read()); // empty buffer
    while (!Serial.available());                 // wait for data
    while (Serial.available() && Serial.read()); // empty buffer again

    // load and configure the DMP
    Serial.println(F("Initializing DMP..."));
    devStatus = mpu.dmpInitialize();

    // supply your own gyro offsets here, scaled for min sensitivity
    mpu.setXGyroOffset(220);
    mpu.setYGyroOffset(76);
    mpu.setZGyroOffset(-85);
    mpu.setZAccelOffset(1788); // 1688 factory default for Jeff's test chip

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
    } else {
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
}



// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void loop() {
  
   //============
   //= WIRELESS =
   //============
   // Receives information from computer
    receiveCharFromComputer();
  
    // Receives information from arduino
    receiveCharFromArduino();

    if (computerChar == '1') {
      digitalWrite(RXLED, HIGH);
      //TXLED;
    } else if (computerChar == '0') {
      digitalWrite(RXLED, LOW);
      //TXLED0;
    }
  
    //===================
    //= READING FROM MPU=
    //===================
    
    // first, we make sure the MPU is working
    // if programming failed, don't try to do anything
    if (!dmpReady) return;

    // wait for MPU interrupt or extra packet(s) available
    while (!mpuInterrupt && fifoCount < packetSize) {
      // do nothing
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
    } else if (mpuIntStatus & 0x02) {
        // wait for correct available data length, should be a VERY short wait
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

        // read a packet from FIFO
        mpu.getFIFOBytes(fifoBuffer, packetSize);
        
        // track FIFO count here in case there is > 1 packet available
        // (this lets us immediately read more without waiting for an interrupt)
        fifoCount -= packetSize;

        //=================
        //= TIMING THROWS =
        //=================
        
        // set aa to current non-gravitational acceleration (a vector with x, y, and z components)
        mpu.dmpGetAccel(&aa, fifoBuffer);
        
        // calculate magnitude of the non-gravitational acceleration it feels from aa.
        // closer to zero -> closer to freefall
        // 1L used to cast the value in the square root as a long (so it can go greater than 32,768), (int) brings it back to int
        int magnitude = (int) sqrt(1L*aa.x*aa.x+1L*aa.y*aa.y+1L*aa.z*aa.z);
    
        // print on the serial, for testing purposes
        Serial.println(magnitude); 
        
        // if it's feeling very little non-gravitational acceleration (say, below 1000) and we haven't set freeFalling to true
        // yet, set it to true and set throwStartTime. The !freeFalling part is to make it only change throwStartTime once
        // per throw, rather than changing over and over as it's in midair.
       if (abs(magnitude)<1000 && !freeFalling) { 
              freeFalling = true;
              throwStartTime = millis();  // millis() is the current time since program start in milliseconds
              Serial.println("Free fall started!"); // for debugging purposes
              Serial1.println("Free fall started!"); // for wireless debugging purposes
       }
       
       // if it feels a strong non-gravitational acceleration (here, we chose a threshold of 3000) and we haven't set freeFalling back to false
        // yet, set it to false, and set throwEndTime and lastThrowDuration.
        if (freeFalling && abs(magnitude)>3000) {
              freeFalling = false;
              throwEndTime = millis();
              lastThrowDuration = throwEndTime - throwStartTime;
              Serial.println("Free fall stopped. Duration:"); // for debugging purposes
              Serial.println(lastThrowDuration); // for debugging purposes
              Serial1.println("Free fall stopped. Duration:"); // for wireless debugging purposes
              Serial1.println(lastThrowDuration); // for wireless debugging purposes
          }
     
        // The below commented-out code uses the built-in getIntFreefallStatus function, which would be more ideal, but 
        // I haven't gotten it to ever get called. Maybe the threshold is too low, and we can fix that with void MPU6050::setFreefallDetectionThreshold(uint8_t threshold) {
// Maybe we need to drop it
        // without the USB cord attached?  
//              if (mpu.getIntFreefallStatus() && !freeFalling) {
//                    freeFalling = true;
//                    throwStartTime = millis();
//                    //Color -> Red or something
//                    Serial.println("Aha!");
//                }
//                if (freeFalling && !mpu.getIntFreefallStatus()) {
//                    freeFalling = false;
//                    throwEndTime = millis();
//                    lastThrowDuration = throwEndTime - throwStartTime;
//                    Serial.println(lastThrowDuration);
//                }

      
        // blink LED to indicate activity
        blinkState = !blinkState;
        digitalWrite(LED_PIN, blinkState);
        
    }
}

/**
Receive a character from the computer.
*/
void receiveCharFromComputer() {
  if (Serial1.available()) {
    computerChar = (char) Serial1.read();
    Serial.print(computerChar);
  }
}

/**
Receive a character from the Arduino.
*/
void receiveCharFromArduino() {
  if (Serial.available()) {
    arduinoChar = (char) Serial.read();
    Serial1.print(arduinoChar);
  }
}
