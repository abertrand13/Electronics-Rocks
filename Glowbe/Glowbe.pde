//--------------------------------------------------------------
//
// Author: Derin Dutz and Matt Mistele
//
// Date: 03.14.14
//
// Description: Glowbe Processing Sketch
//
//--------------------------------------------------------------

// Imports
import processing.serial.*;
import ddf.minim.*;
import controlP5.*;
import com.onformative.yahooweather.*;

ControlP5 cp5;
MultiList l;
Button ms;
Button pa; // play animation
Button sa; // stop animation
Slider al; // animation length
ColorPicker cp;
YahooWeather weather;
int updateIntervallMillis = 30000;

String serialString = "";

Serial myPort;        // The serial port

int WINDOW_WIDTH = 400;
int WINDOW_HEIGHT = 700;

int BALL_IMG_WIDTH = 55;
int BALL_IMG_HEIGHT = 55;
int BALL_IMG_X = 12;
int BALL_IMG_Y = WINDOW_HEIGHT - BALL_IMG_HEIGHT - 14;

int TEXT_X = BALL_IMG_WIDTH + 22;

int SECONDARY_LIST_WIDTH = 212;

// Temperature Mode
char TEMPERATURE_MODE = '5';

// LED Values
int redVal = 255;
int greenVal = 255;
int blueVal = 255;

// WOEID (Web Temperature Location)
// 23511612 = the WOEID of Stanford University
// use this site to find out about your WOEID : http://woeid.rosselliot.co.nz/
int WOEID = 23511612;

// Current Web Temperature
int currentTemp = 0;

// Background image
PImage bgImg;
PImage colorBallImg;

// Boolean value that says whether the color picker is active
boolean colorPickerActive = false;
boolean colorKeyframesActive = false;
// Global variables/constants for the color animation GUI
int showLength = 100;
boolean animating = false;
int frame;
int lastKeyframeIndices[] = new int[3];
int firstBoxX = 20;
int firstBoxY = 300;
int boxWidth = 200;
int boxHeight = 100;
boolean isClicking = false;
boolean draggingKeyframe = false;
Keyframe selectedKeyframe;
int selectedKeyframeIndex;
int selectedBoxIndex;
KeyframeBox[] keyframeBoxes;

/*
Sets the window to be full screen. Called automatically when the program begins.
 */
/*
boolean sketchFullScreen() {
 return true;
 }
 */

/*
 * Sets up the program. Called automatically when the program begins.
 */
void setup() { 
  // Set window size
  size(WINDOW_WIDTH, WINDOW_HEIGHT);

  // Set text font
  textFont(createFont("Arial", 14));

  // Sets up the Serial port. Enter the name of your usb cable here by checking which usb is being used.   
  //myPort = new Serial(this, "/dev/tty.usbserial-A702NXEJ", 9600);   
  myPort = new Serial(this, "/dev/tty.usbserial-A702O078", 9600);    
  myPort.bufferUntil('\n');      // Buffers until the Serial sends a new line character   

  // Main GUI Controller
  cp5 = new ControlP5(this);

  // Mode Selection Button
  ms = cp5.addButton("GoToModeSelection").setLabel("Go To Mode Selection").setValue(0).setPosition(10, 10).setSize(95, 20);

  // Mode Select Multilist
  l = cp5.addMultiList("myList", 120, 10, 60, 20); 

  // Multilist Setup
  MultiListButton levelSelect;
  levelSelect = l.add("Mode Select", -1);
  levelSelect.add("mode0", 0).setLabel("Mode 0: Acceleration Fade").setWidth(SECONDARY_LIST_WIDTH);
  levelSelect.add("mode1", 1).setLabel("Mode 1: Rotation Fade").setWidth(SECONDARY_LIST_WIDTH);
  levelSelect.add("mode2", 2).setLabel("Mode 2: Acceleration Fade Based On Temperature").setWidth(SECONDARY_LIST_WIDTH);
  levelSelect.add("mode3", 3).setLabel("Mode 3: Red Blue Fade").setWidth(SECONDARY_LIST_WIDTH);
  levelSelect.add("mode4", 4).setLabel("Mode 4: Throw Air Time Fade").setWidth(SECONDARY_LIST_WIDTH);
  levelSelect.add("mode5", 5).setLabel("Mode 5: Acceleration Flashing").setWidth(SECONDARY_LIST_WIDTH);
  levelSelect.add("mode6", 6).setLabel("Mode 6: Light Show").setWidth(SECONDARY_LIST_WIDTH);
  levelSelect.add("mode7", 7).setLabel("Mode 7: Web Temperature Fade").setWidth(SECONDARY_LIST_WIDTH);
  levelSelect.add("mode8", 8).setLabel("Mode 8: Acceleration Light Show").setWidth(SECONDARY_LIST_WIDTH);

  // Play animation Button
  pa = cp5.addButton("playAnimation")
    .setValue(0)
      .setCaptionLabel("Play!")
        .setPosition(firstBoxX, firstBoxY-90)
          .setSize(boxWidth, 25);

  // Stop Animation Button
  sa = cp5.addButton("stopAnimation")
    .setValue(0)
      .setCaptionLabel("Stop")
        .setPosition(firstBoxX, firstBoxY-60)
          .setSize(boxWidth, 25);

  // Animation Length Slider
  cp5.addSlider("lengthSlider")
    .setCaptionLabel("Animation length")
      .setColorCaptionLabel(0)
        .setPosition(firstBoxX, firstBoxY-30)
          .setSize(boxWidth/2, 25)
            .setValue(200)
              .setRange(10, 1000);

  // Weather
  weather = new YahooWeather(this, WOEID, "c", updateIntervallMillis);

  // Load background image
  bgImg = loadImage("background.jpg");
  bgImg.resize(WINDOW_WIDTH, WINDOW_HEIGHT);

  // Load glowbe image
  colorBallImg = loadImage("colorBall.png");
  colorBallImg.resize(BALL_IMG_WIDTH, BALL_IMG_HEIGHT);

  // Set up the boxes for color animation by keyframes later
  keyframeBoxes =  new KeyframeBox[3];
  keyframeBoxes[0] = new KeyframeBox(firstBoxX, firstBoxY, boxWidth, boxHeight, showLength, color(255, 0, 0));
  keyframeBoxes[1] = new KeyframeBox(firstBoxX, firstBoxY+boxHeight+10, boxWidth, boxHeight, showLength, color(0, 255, 0));
  keyframeBoxes[2] = new KeyframeBox(firstBoxX, firstBoxY+2*boxHeight+20, boxWidth, boxHeight, showLength, color(0, 0, 255));
}

/*
 * Main drawing loop: called automatically when the program begins.
 */
void draw() {
  // Updates weather
  weather.update();

  // Sets background.
  if (colorPickerActive) {
    if (colorKeyframesActive && animating) {
      animate();
    }
    else
      background(cp.getColorValue());

    // Display Keyframe boxes  
    fill(10, 10, 10);
    for (int i=0; i<3; i++) {
      KeyframeBox kfBox = keyframeBoxes[i];
      rect(kfBox.x, kfBox.y, kfBox.boxWidth, kfBox.boxHeight);
    }

    // Display animation frame line
    if (animating) {
      stroke(255); 
      for (int i=0; i<3; i++) {
        KeyframeBox kfBox = keyframeBoxes[i];
        line(timeToX(frame, kfBox), kfBox.y, timeToX(frame, kfBox), kfBox.y+kfBox.boxHeight);
      }
    }
    // Deleting keyframes
    if (keyPressed) {
      if (key == BACKSPACE || key == DELETE) {
        if (selectedKeyframe != null) {
          draggingKeyframe = false;
          selectedKeyframe = null; 
          keyframeBoxes[selectedBoxIndex].keyframes.remove(selectedKeyframeIndex); 
          animating = false; // to prevent outOfBoundsExceptions, for now
        }
      }
    }
    // Displaying and manipulating the keyframes
    for (int b = 0; b < 3; b++) {
      // Draw leftmost keyframe
      Keyframe leftKf = keyframeBoxes[b].keyframes.get(0);
      leftKf.display();

      // Draw and/or manipulate each successive keyframe
      for (int i = 1; i < keyframeBoxes[b].keyframes.size(); i++) {
        Keyframe rightKf = keyframeBoxes[b].keyframes.get(i);
        // if this keyframe is to the left of previous, swap them in the arraylist!
        if (rightKf.x < leftKf.x) {
          keyframeBoxes[b].keyframes.remove(i-1);
          keyframeBoxes[b].keyframes.add(i, leftKf);
          if (selectedBoxIndex == b && selectedKeyframeIndex == i-1)
            selectedKeyframeIndex++;
          else if (selectedBoxIndex == b && selectedKeyframeIndex == i)
            selectedKeyframeIndex--;
        }

        // Clicked on an existing keyframe in the middle -> select it
        // The !draggingKeyframe part is to prevent selecting many at once as you drag the cursor around
        if (mousePressed && dist(mouseX, mouseY, rightKf.x, rightKf.y) < rightKf.w && i < keyframeBoxes[b].keyframes.size() - 1 && !draggingKeyframe) {
          rightKf.select();
          draggingKeyframe = true;
          if (selectedKeyframe != null)
            selectedKeyframe.deselect();
          selectedKeyframe = rightKf;
          selectedKeyframeIndex = i;
          selectedBoxIndex = b;
        }

        if (rightKf.beingDragged) {
          // Check if the mouse is in the box for keyframes 
          if (mouseInBox(keyframeBoxes[b])) {
            rightKf.x = mouseX;
            rightKf.y = mouseY;
            rightKf.time = xToTime(mouseX, keyframeBoxes[b]);
            rightKf.value = yToValue(mouseY, keyframeBoxes[b]);
          }
          else { // Stop dragging it!
            rightKf.stopDragging();
            draggingKeyframe = false;
          }
        }

        stroke(keyframeBoxes[b].keyframeColor);
        line(leftKf.x, leftKf.y, rightKf.x, rightKf.y); // Draw line between this keyframe and the previous
        rightKf.display();
        leftKf = rightKf; // So drawing the lines works right
      }
      // Clicked in the box but not on a keyframe -> create and select a new keyframe
      if (mousePressed && !draggingKeyframe && mouseInBox(keyframeBoxes[b])) {
        Keyframe newKeyframe = new Keyframe(xToTime(mouseX, keyframeBoxes[b]), yToValue(mouseY, keyframeBoxes[b]), mouseX, mouseY, keyframeBoxes[b].keyframeColor);
        newKeyframe.select();
        draggingKeyframe = true;
        if (selectedKeyframe != null)
          selectedKeyframe.deselect();
        selectedKeyframe = newKeyframe;
        //animating = false; to prevent outOfBoundsExceptions, for now

        // Find its proper index according to its x position
        for (int i = 1; i < keyframeBoxes[b].keyframes.size(); i++) {
          Keyframe kf = keyframeBoxes[b].keyframes.get(i);
          if (kf.x > newKeyframe.x) {
            keyframeBoxes[b].keyframes.add(i, newKeyframe);
            selectedKeyframeIndex = i;
            selectedBoxIndex = b;
            break;
          }
        }
      }
    }
    // Letting go of keyframes
    if (!mousePressed && draggingKeyframe) {
      draggingKeyframe = false;
      selectedKeyframe.stopDragging();
      // need to set selectedKeyframe to null?
    }
  } 
  else {
    background(bgImg);
  }

  image(colorBallImg, BALL_IMG_X, BALL_IMG_Y);


  // Textual weather information
  fill(0);
  textSize(9);
  currentTemp = int(weather.getTemperature() * 1.8) + 32;
  text("Current Temperature: "+currentTemp, TEXT_X, WINDOW_HEIGHT - 60);
  text("City: "+weather.getCityName()+"; Region: "+weather.getRegionName()+"; Country: "+weather.getCountryName(), TEXT_X, WINDOW_HEIGHT - 40);
  text("Last updated: "+weather.getLastUpdated(), TEXT_X, WINDOW_HEIGHT - 20);
}

/*
 * Called when a Serial event occurs: gets the handle position data and adjusts the paddle positions on the screen accordingly.
 */
void serialEvent(Serial myPort) {
  // Reads in data from the Serial until a new line. This data comes from the Arduino program.
  serialString = myPort.readStringUntil('\n');

  // Writes temperature to port when "GetTemperature" is received
  if (serialString.charAt(0) == 'G') {
    myPort.write(String.valueOf(currentTemp));
    myPort.write("\n");
  }
}

/*
 * Called when a controller event occurs in the GUI.
 */
public void controlEvent(ControlEvent c) {
  // Extracts and writes the RGB values to port when the ColorPicker changes value
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
    for (int b=0; b<3; b++) {
      KeyframeBox kfBox = keyframeBoxes[b];
      int colorComponent = (int) c.getArrayValue(b);
      int lastIndex = kfBox.keyframes.size()-1;
      kfBox.keyframes.get(0).value = colorComponent;
      kfBox.keyframes.get(0).y = valueToY(colorComponent, kfBox);
      kfBox.keyframes.get(lastIndex).value = colorComponent;
      kfBox.keyframes.get(lastIndex).y = valueToY(colorComponent, kfBox);
    }
  } 
  // Writes the selected mode to port when the appropriate button is clicked
  else if (c.isFrom(l) && c.value() != -1) {
    if (int(c.value()) == 6 || int(c.value()) == 8) {
      // Color picker
      cp = cp5.addColorPicker("picker").setPosition(60, 100).setColorValue(color(255, 128, 0, 128));
      // Play Animation button
      pa = cp5.addButton("playAnimation")
        .setValue(0)
          .setCaptionLabel("Play!")
            .setPosition(firstBoxX, firstBoxY-90)
              .setSize(boxWidth, 25);
      // Stop Animation button
      sa = cp5.addButton("stopAnimation")
        .setValue(0)
          .setCaptionLabel("Stop")
            .setPosition(firstBoxX, firstBoxY-60)
              .setSize(boxWidth, 25);
      // Animation length slider
      al = cp5.addSlider("lengthSlider")
        .setCaptionLabel("Animation length")
          .setColorCaptionLabel(0)
            .setPosition(firstBoxX, firstBoxY-30)
              .setSize(boxWidth/2, 25)
                .setValue(200)
                  .setRange(10, 1000);
      colorPickerActive = true;
      colorKeyframesActive = true;
    }
    myPort.write(String.valueOf(int(c.value())));
    myPort.write("\n");
  } 
  // Writes to go to mode selection when the approriate button is clicked
  else if (c.isFrom(ms)) {
    if (colorPickerActive) {
      cp.remove();
      colorPickerActive = false;
    }
    myPort.write('r');
    myPort.write("\n");
  }
  // Button pressed -> play color animation
  else if (c.isFrom(pa)) {
    animating = true;
    frame = 0;
    lastKeyframeIndices[0] = 0;
    lastKeyframeIndices[1] = 0;
    lastKeyframeIndices[2] = 0;
  }
  // Button pressed -> stop color animation
  else if (c.isFrom(sa)) {
    animating = false;
    frame = 0;
  }
  else if (c.isFrom(al)) {
    showLength = (int)c.value();
    animating = false; // to prevent outOfBoundsExceptions, for now
    for (int b=0; b < 3; b++) {
      for (int i=0; i < keyframeBoxes[b].keyframes.size(); i++) {
        Keyframe kf = keyframeBoxes[b].keyframes.get(i);
        kf.time = xToTime(kf.x, keyframeBoxes[b]);
      }
    }
  }
}

void animate() {
  int rgb[] = new int[3];
  myPort.write("Colors: ");
  for (int b = 0; b < 3; b++) {
    Keyframe lastKeyframe = keyframeBoxes[b].keyframes.get(lastKeyframeIndices[b]);
    Keyframe nextKeyframe = keyframeBoxes[b].keyframes.get(lastKeyframeIndices[b]+1);
    int duration = nextKeyframe.time-lastKeyframe.time;
    int timeSinceLast = frame - lastKeyframe.time;
    float progress = ((float)timeSinceLast)/duration;
    rgb[b] = (int) lerp(lastKeyframe.value, nextKeyframe.value, progress); 

    while (lastKeyframeIndices[b] < keyframeBoxes[b].keyframes.size () - 1  && frame > keyframeBoxes[b].keyframes.get(lastKeyframeIndices[b]+1).time) {
      lastKeyframeIndices[b]++;
    }
    myPort.write(String.valueOf(rgb[b]));
    myPort.write(" ");
  }
  myPort.write("\n");
  background(rgb[0], rgb[1], rgb[2]);  
  frame++;
  if (frame >= showLength) {
    frame = 0;
    lastKeyframeIndices[0] = 0;
    lastKeyframeIndices[1] = 0;
    lastKeyframeIndices[2] = 0;
  }
}

// Keyframe manipulation helper methods

int xToTime(float x, KeyframeBox kfBox) {
  return (int) ((x - kfBox.x) /  kfBox.boxWidth * showLength);
}

int yToValue(float y, KeyframeBox kfBox) {
  return (int) ((kfBox.y + kfBox.boxHeight - y) / kfBox.boxHeight * 255);
}

float timeToX(int time, KeyframeBox kfBox) {
  return kfBox.x + kfBox.boxWidth * ((float) time)/showLength;
}

float valueToY(int value, KeyframeBox kfBox) {
  return kfBox.y + kfBox.boxHeight * (1- ((float) value)/255);
}

boolean mouseInBox(KeyframeBox kfBox) {
  return mouseX > kfBox.x && mouseX < kfBox.x + kfBox.boxWidth && mouseY > kfBox.y && mouseY < kfBox.y + kfBox.boxHeight;
}

