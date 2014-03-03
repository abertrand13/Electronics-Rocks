/* 
File: hotPotato.ino
Author: Tucker
Date: 2/28/14

Code for the "hot potato" feature.
The basic idea: Get a group of people together and start playing catch with the ball (or multiple balls?).
The ball will start blinking blue and will blink more and more rapidly as the game progresses.
Eventually, when the game is over, the ball turns red. Whoever's holding on to it at that point is out.

relies on the delay funtction for timing. calls "isFreeFalling" to check whether or not someone is holding the
ball when the game ends.

If all the variables in hotPotato() are declared globally, you can effectively remove the main while loop with an if-else statement
and do the looping through void loop(). This might be desirable if hotPotato() is to be run alongside other functions in void loop(),
e.g. a function that watches for some other input from the Xbees or a Pebble-style input from the user
*/


//RGB LED pins
int ledPins[] = {9, 10, 11}; //9 = red Pin, 10 = green Pin, 11 = blue pin

void setup(){
  for(int i = 0; i < 3; i++){
   pinMode(ledPins[i], OUTPUT);
  }
  randomSeed(analogRead(0)); //initializes the random() function with analog noise from an unused pin
 Serial.begin(9600); 
}

void loop() {
  hotPotato();
}

void hotPotato() {
  
  int t_on = 100; //time (in ms) that the led should be on (per blink)
  int t_off0 = 1000; //time (in ms) that the led should be off at the loop's start
  int t_off; //time that the led should be off (per blink). this will change over the course of the loop
  
  int rON[] = {255,0,0}; //lights red LED
  int bON[] = {0,0,255}; //lights blue LED
  int OFF[] = {0,0,0};   
  
  int duration = random(10000,30000); //length of one hot-potato game in ms. chosen randomly to be between 10 and 30 seconds
  int t_lastSwitch = 0;
  int t = 0;
  
  Serial.println(duration);
  
  boolean isOn = true;
  setColor(ledPins, bON);
  
  while (t < duration) {
    
    //these if statements blink the blue led
    if (isOn){
      if (t - t_lastSwitch > t_on) { //if the led's on, check if it's time to turn it off
        isOn = false;
        t_lastSwitch = t;
      }
    } else {//if the led's off, check if it's time to turn it on
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
 
 delay(5000); //could eventually be replaced with some function that looks for an input from the user
              // to start a new game
}

//for common anode led
void setColor(int* led, int* color){
 for(int i = 0; i < 3; i++){
   analogWrite(led[i], 255-color[i]); //note: a low output turns the led on
 }
}

//one possible test to see whether the ball is in free fall. Assumes the MPU 6050 has 
// already been properly initialized and that "VectorInt16 aa;" has been declared
boolean isFreeFalling() {
  int xAcc = aa.x;
  int yAcc = aa.y;
  int zAcc = aa.z;
  
  int a = sqrt(pow(xAcc,2) + pow(yAcc,2) + pow(zAcc,2)); //magnitude of the 
  
  return (a < 1000) ? true : false; //the 1000 here is essentially arbi
}
