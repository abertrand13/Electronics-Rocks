//red to blue. Might need to change color more often.
//seems to work, but not tested with avg time function and real free fall
//note: my led is lit up at 0 and off at 255. If this is also true
//  for led in juggling ball, must switch red and blue variables.

//function needs pin number of red, blue, and time (in ms) fall expected to take.
void redToBlue(int time,int redPin,int bluePin) {
  int red; //will give how much red output from scale of 0 to 255
  int blue;
  
  unsigned long startTime=millis();
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


