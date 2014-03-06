
int ledPins[] = {9, 10, 11};
//9 = Red
//10 = Blue
//11 = Green

void setup() {
  Serial.begin(9600);
  
  for(int i = 0;i < 3;i++) {
    pinMode(ledPins[i], OUTPUT);
  }
}

void loop() {
  //should go red green blue white;
  
  //int value = analogRead(sensePin)/4; //0-255
  //draw(255-value, 255, value); //fade red to blue
  
  //Common cathode (common power)
  draw(255,0,0);
  delay(500);
  draw(0,255,0);
  delay(500);
  draw(0,0,255);
  delay(500);
  draw(255, 255, 255);
  delay(500);
  
  
  //draw(0,255,0);
  
  //common anode (common ground)
  /*draw(0,255,255);
  delay(500);
  draw(255,0,255);
  delay(500);
  draw(255,255,0);
  delay(500);
  draw(0,0,0);
  delay(500);*/
}

void draw(int r, int g, int b) {
  analogWrite(color('r'), r);
  analogWrite(color('g'), g);
  analogWrite(color('b'), b);
}

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
