/* 
* File: spinToGlow.ino
* Author: Tucker
* Date: 2/24/14
*
* Contains functions for implementing the "spin to glow" feature,
* where the ball lights up according to how fast it's spinning.
* Calls angularSpeed() to get the ball's current angular velocity 
*/




//RGB LED pins
int ledPins[] = {9, 10, 11}; //9 = red Pin, 10 = green Pin, 11 = blue pin

//defines what angular speed, in rad/s, gives maximum output.
double MAXW = 20; //corresponds to an angular speed of about 3 rotations/sec 


//test code for demoing purposes. nothing in setup() or loop() is essential, besides declaring the LED pins as outputs.
double k = 0;

void setup(){
  for(int i = 0; i < 3; i++){
   pinMode(ledPins[i], OUTPUT);   //Set the three LED pins as outputs
  }
 Serial.begin(9600); 
}

void loop(){

  spinToGlow();
  //spinBlueToRed();
  Serial.println(k);
  k += 0.1;
  delay(100);
}


/* Writes to the RBG LED
   led - a three element array defining the three color pins (led[0] = redPin, led[1] = greenPin, led[2] = bluePin)
   color - a three element array  containing the write values for the red (0), green (1), and blue (2) leds.
   0 is off, 255 is on 
*/
void setColor(int* led, int* color){
 for(int i = 0; i < 3; i++){
   analogWrite(led[i], 255-color[i]);
 }
}


void spinToGlow() { //makes the led glow green according to how fast the ball is spinning
  
  double w = angularSpeed();
  int ledValue = (int)mapDouble(w, 0, MAXW, 0, 255);
  ledValue = constrain(ledValue, 0, 255);
  
  int writeValue[] = {0, ledValue, 0}; 
  setColor(ledPins, writeValue); 
}

void spinBlueToRed() { //when not spinning, the led is blue. the led glows more red as the ball spins faster
  
  double w = angularSpeed();
  int ledValue = (int)mapDouble(w, 0, MAXW, 0, 255);
  ledValue = constrain(ledValue, 0, 255);
  
  int writeValue[] = {ledValue, 0, 255-ledValue}; 
  setColor(ledPins, writeValue);  
}

//my implementation of the map function for floating points
double mapDouble(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//just a dummy function here for demoing purposes
double angularSpeed() {
  return k;
}
