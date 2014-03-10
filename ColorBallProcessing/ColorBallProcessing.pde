//--------------------------------------------------------------
//
// Author: Derin Dutz
//
// Date: 03.10.14
//
// Description: Color Ball Processing
//
// Important Information:
//   Admin Keyboard Controls: 
//      'r' = go to mode selection
//      '1' - '9' = set mode
//--------------------------------------------------------------

// Imports
import processing.serial.*;
import ddf.minim.*;
import controlP5.*;
import com.onformative.yahooweather.*;

ControlP5 cp5;
MultiList l;
Button ms;
ColorPicker cp;
YahooWeather weather;
int updateIntervallMillis = 30000;

String serialString = "";

Serial myPort;        // The serial port

int WINDOW_WIDTH = 400;
int WINDOW_HEIGHT = 600;

int SECONDARY_LIST_WIDTH = 150;

char TEMPERATURE_MODE = '5';

int redVal = 255;
int greenVal = 255;
int blueVal = 255;

int currentTemp = 0;

PImage bgImg;

boolean colorPickerActive = false;

/*
Sets the window to be full screen. Called automatically when the program begins.
 */
/*
boolean sketchFullScreen() {
 return true;
 }
 */

/*
Sets up the game. Called automatically when the program begins.
 */
void setup() { 
  size(WINDOW_WIDTH, WINDOW_HEIGHT);
  textFont(createFont("Arial", 14));
  // Sets up the Serial port. Enter the name of your usb cable here by going to the Arduino program, then clicking on tools, then Serial Port, and then checking which usb is being used   
  myPort = new Serial(this, "/dev/tty.usbserial-A702NXEJ", 9600);    
  myPort.bufferUntil('\n');      // Buffers until the Serial sends a new line character   

  cp5 = new ControlP5(this);

  // Color picker
  //cp = cp5.addColorPicker("picker").setPosition(60, 100).setColorValue(color(255, 128, 0, 128));

  // Mode Selection Button
  ms = cp5.addButton("GoToModeSelection").setLabel("Go To Mode Selection").setValue(0).setPosition(10, 10).setSize(95, 20);

  // Multilist
  l = cp5.addMultiList("myList", 120, 10, 60, 20); 

  MultiListButton levelSelect;
  levelSelect = l.add("Mode Select", -1);
  levelSelect.add("mode0", 0).setLabel("Mode 0: Acceleration Fade").setWidth(SECONDARY_LIST_WIDTH);
  levelSelect.add("mode1", 1).setLabel("Mode 1: Rotation Fade").setWidth(SECONDARY_LIST_WIDTH);
  levelSelect.add("mode2", 2).setLabel("Mode 2: Hot Potato").setWidth(SECONDARY_LIST_WIDTH);
  levelSelect.add("mode3", 3).setLabel("Mode 3: Fade Red Blue").setWidth(SECONDARY_LIST_WIDTH);
  levelSelect.add("mode4", 4).setLabel("Mode 4: Light Cycle").setWidth(SECONDARY_LIST_WIDTH);
  levelSelect.add("mode5", 5).setLabel("Mode 5: Color Picker").setWidth(SECONDARY_LIST_WIDTH);
  levelSelect.add("mode6", 6).setLabel("Mode 6: Sensor Temperature").setWidth(SECONDARY_LIST_WIDTH);
  levelSelect.add("mode7", 7).setLabel("Mode 7: Web Temperature").setWidth(SECONDARY_LIST_WIDTH);
  levelSelect.add("mode8", 8).setLabel("Mode 8").setWidth(SECONDARY_LIST_WIDTH);

  // 23511612 = the WOEID of Stanford University
  // use this site to find out about your WOEID : http://sigizmund.info/woeidinfo/
  weather = new YahooWeather(this, 23511612, "c", updateIntervallMillis);
  
  bgImg = loadImage("background.jpg");
  bgImg.resize(WINDOW_WIDTH, WINDOW_HEIGHT);
}

/*
Main drawing loop: called automatically when the program begins.
 */
void draw() {
  weather.update();

  if (colorPickerActive) {
    background(cp.getColorValue());
  } else {
    background(bgImg);
  }
    
  
  fill(0);
  //textSize(12);
  //text(serialString, 20, 50);

  textSize(9);
  currentTemp = int(weather.getTemperature() * 1.8) + 32;
  text("Current Temperature: "+currentTemp, 20, WINDOW_HEIGHT - 60);
  text("City: "+weather.getCityName()+"; Region: "+weather.getRegionName()+"; Country: "+weather.getCountryName(), 20, WINDOW_HEIGHT - 40);
  text("Last updated: "+weather.getLastUpdated(), 20, WINDOW_HEIGHT - 20);
}

/*
Called when a Serial event occurs: gets the handle position data and adjusts the paddle positions on the screen accordingly.
 */
void serialEvent(Serial myPort) {
  // Reads in data from the Serial until a new line. This data comes from the Arduino program and represents the player or the x position of the handle
  serialString = myPort.readStringUntil('\n');

  if (serialString.charAt(0) == 'G') {
    myPort.write(String.valueOf(currentTemp));
    myPort.write("\n");
  }
}

public void controlEvent(ControlEvent c) {
  // when a value change from a ColorPicker is received, extract the ARGB values
  // from the controller's array value
  if (c.isFrom(cp)) {
    redVal = int(c.getArrayValue(0));
    myPort.write("Colors: ");
    myPort.write(String.valueOf(redVal));
    myPort.write(" ");
    greenVal = int(c.getArrayValue(1));
    myPort.write(String.valueOf(greenVal));
    myPort.write(" ");
    blueVal = int(c.getArrayValue(2));
    myPort.write(String.valueOf(blueVal));
    myPort.write(" ");
    myPort.write("\n");
    int a = int(c.getArrayValue(3));
    color col = color(redVal, greenVal, blueVal, a);
  } 
  else if (c.isFrom(l) && c.value() != -1) {
    if (int(c.value()) == 5) {
      // Color picker
      cp = cp5.addColorPicker("picker").setPosition(60, 100).setColorValue(color(255, 128, 0, 128));
      colorPickerActive = true;
    }
    myPort.write(String.valueOf(int(c.value())));
    myPort.write("\n");
  } 
  else if (c.isFrom(ms)) {
    if (colorPickerActive) {
      cp.remove();
      colorPickerActive = false;
    }
    myPort.write('r');
    myPort.write("\n");
  }
}

