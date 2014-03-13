import processing.core.*; 
import processing.data.*; 
import processing.event.*; 
import processing.opengl.*; 

import controlP5.*; 

import java.util.HashMap; 
import java.util.ArrayList; 
import java.io.File; 
import java.io.BufferedReader; 
import java.io.PrintWriter; 
import java.io.InputStream; 
import java.io.OutputStream; 
import java.io.IOException; 

public class LightShow extends PApplet {

/*
 * Lets you make a light show!
 *
 */
 


ControlP5 cp5;
Textlabel instructionsLabel;

int showLength = 100;

boolean animating = false;
int frame;
int lastKeyframeIndex;
 
int redBoxX = 20;
int redBoxY = 300;
int boxWidth = 200;
int boxHeight = 200;
boolean isClicking = false;
boolean draggingKeyframe = false;
Keyframe selectedKeyframe;
int selectedKeyframeIndex;
ArrayList<Keyframe> redKeyframes; // stores keyframes in ascending x order

//ArrayList<Keyframe> greenKeyframes;
//ArrayList<Keyframe> blueKeyframes;

public void setup() {  
  size(360, 640);
  colorMode(RGB, 255);
  noStroke();
  background(255, 255, 255);
  
  redKeyframes = new ArrayList<Keyframe>();
  redKeyframes.add(new Keyframe(0, 255, redBoxX, redBoxY));
  redKeyframes.add(new Keyframe(showLength, 255, redBoxX+boxWidth, redBoxY));
  
  cp5 = new ControlP5(this);
  instructionsLabel = cp5.addTextlabel("label")
    .setText("LIGHT SHOW (currently only available in red)\n*Click anywhere in the black box to create a new\nkeyframe\n*Keyframes can be clicked and dragged to change\nthe animation\n*Delete or Backspace will erase the highlighted\nkeyframe\n*Click PLAY! to test the show")
    .setPosition(10, redBoxY-240)
    .setColorValue(0xfff00000)
    .setFont(createFont("Georgia",15))
    ;
    
  cp5.addButton("playAnimation")
    .setValue(0)
    .setCaptionLabel("Play!")
    .setPosition(redBoxX, redBoxY-90)
    .setSize(boxWidth, 25);
    
  cp5.addButton("stopAnimation")
    .setValue(0)
    .setCaptionLabel("Stop")
    .setPosition(redBoxX, redBoxY-60)
    .setSize(boxWidth, 25);
    
  cp5.addSlider("slider")
    .setCaptionLabel("Animation length")
    .setColorCaptionLabel(0)
    .setPosition(redBoxX, redBoxY-30)
    .setSize(boxWidth/2, 25)
    .setValue(200)
    .setRange(10,1000)
    ;
}

public void draw() {
  // Coloring the box
  if (animating)
    animate();
    
  fill(10, 10, 10);
  rect(redBoxX, redBoxY, boxWidth, boxHeight);
  
  // time ticker
  if (animating) {
    stroke(255); 
    line(timeToX(frame), redBoxY, timeToX(frame), redBoxY+boxHeight);
  }
  
  // Deleting keyframes
  if (keyPressed) {
    if (key == BACKSPACE || key == DELETE) {
      if (selectedKeyframe != null) {
        draggingKeyframe = false;
        selectedKeyframe = null; 
        redKeyframes.remove(selectedKeyframeIndex); 
        animating = false; // to prevent outOfBoundsExceptions, for now
      }
    }
  }
  
  // Draw leftmost keyframe
  Keyframe leftKf = redKeyframes.get(0);
  leftKf.display();
  
  // Draw and/or manipulate each successive keyframe
  for (int i = 1; i < redKeyframes.size(); i++) {
     Keyframe rightKf = redKeyframes.get(i);
    // if this keyframe is to the left of previous, swap them in the arraylist!
    if (rightKf.x < leftKf.x) {
      redKeyframes.remove(i-1);
      redKeyframes.add(i, leftKf);
      if (selectedKeyframeIndex == i-1)
        selectedKeyframeIndex++;
      else if (selectedKeyframeIndex == i)
        selectedKeyframeIndex--;
    }
    
    // Clicked on an existing keyframe in the middle -> select it
    // The !draggingKeyframe part is to prevent selecting many at once as you drag the cursor around
    if (mousePressed && dist(mouseX, mouseY, rightKf.x, rightKf.y) < rightKf.w && i < redKeyframes.size() - 1 && !draggingKeyframe) {
        rightKf.select();
        draggingKeyframe = true;
        if (selectedKeyframe != null)
          selectedKeyframe.deselect();
        selectedKeyframe = rightKf;
        selectedKeyframeIndex = i;
    }
    
    if (rightKf.beingDragged) {
      // Check if the mouse is in the box for keyframes 
      if (mouseInRedBox()) {
            rightKf.x = mouseX;
            rightKf.y = mouseY;
            rightKf.time = xToTime(mouseX);
            rightKf.value = yToValue(mouseY);
        }
        else { // Stop dragging it!
          rightKf.stopDragging();
          draggingKeyframe = false;
        }
    }
   
    stroke(255,0,0);
    line(leftKf.x, leftKf.y, rightKf.x, rightKf.y); // Draw line between this keyframe and the previous
    rightKf.display();
    leftKf = rightKf; // So drawing the lines works right
    
  }
  
  // Clicked in the box but not on a keyframe -> create and select a new keyframe
  if (mousePressed && !draggingKeyframe && mouseInRedBox()) {
    Keyframe newKeyframe = new Keyframe(xToTime(mouseX), yToValue(mouseY), mouseX, mouseY);
    newKeyframe.select();
    draggingKeyframe = true;
    if (selectedKeyframe != null)
      selectedKeyframe.deselect();
    selectedKeyframe = newKeyframe;
    //animating = false; to prevent outOfBoundsExceptions, for now
 
    // Find its proper index according to its x position
    for (int i = 1; i < redKeyframes.size(); i++) {
       Keyframe kf = redKeyframes.get(i);
       if (kf.x > newKeyframe.x) {
          redKeyframes.add(i, newKeyframe);
          selectedKeyframeIndex = i;
          break; 
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

// called when "Play!" button pressed
public void playAnimation(int value) {
  animating = true;
  frame = 0;
  lastKeyframeIndex = 0;
}

// called when "Stop" button pressed
public void stopAnimation(int value) {
  animating = false;
  frame = 0;
}

public void slider(int theDuration) {
  showLength = theDuration;
  animating = false; // to prevent outOfBoundsExceptions, for now
  for (int i=0; i < redKeyframes.size(); i++) {
     Keyframe kf = redKeyframes.get(i);
     kf.time = xToTime(kf.x);
  }
}

public void animate() {
  Keyframe lastKeyframe = redKeyframes.get(lastKeyframeIndex);
  Keyframe nextKeyframe = redKeyframes.get(lastKeyframeIndex+1);
  int duration = nextKeyframe.time-lastKeyframe.time;
  int timeSinceLast = frame - lastKeyframe.time;
  float progress = ((float)timeSinceLast)/duration;
  int r = (int) lerp(lastKeyframe.value, nextKeyframe.value, progress); 
  
  while (lastKeyframeIndex < redKeyframes.size() - 1  && frame > redKeyframes.get(lastKeyframeIndex+1).time) {
    lastKeyframeIndex++;
  }
  
  background(r, 0, 0);
  frame++;
  if (frame >= showLength) {
    frame = 0;
    lastKeyframeIndex = 0;
  }
}

public int xToTime(float x) {
   return (int) ((x - redBoxX) / boxWidth * showLength);
}

public int yToValue(float y) {
   return (int) ((redBoxY + boxHeight - y) / boxHeight * 255);
}

public float timeToX(int time) {
   return redBoxX + boxWidth * ((float) time)/showLength;
}

public float valueToY(int value) {
   return redBoxY + boxHeight * (1- ((float) value)/255);
}

public boolean mouseInRedBox() {
  return mouseInRedBoxX() && mouseInRedBoxY();
}

public boolean mouseInRedBoxX() {
  return mouseX > redBoxX && mouseX < redBoxX + boxWidth;
}

public boolean mouseInRedBoxY() {
  return mouseY > redBoxY && mouseY < redBoxY + boxHeight;
}

// Simple keyframe point class

class Keyframe {
  
  int time; // in milliseconds
  int value; // b/w 0 and 255
  float x; // display x
  float y; // display y
  float w; // display width (radius)
  float r; // its own color
  float g;
  float b;
  boolean beingDragged;
  boolean selected;
  
  
  Keyframe(int time0, int value0, float x0, float y0) {
    time = time0;
    value = value0;
    x = x0;
    y = y0;
    w = 5;
    r = 255;
    g = 0;
    b = 0;
    beingDragged = false;
    selected = false;
  }
  

  public void select() {
    beingDragged = true;
    selected = true;
  }
  
  public void stopDragging() {
    beingDragged = false;
  }
  
  public void deselect() {
    beingDragged = false;
    selected = false;  
  }
  

  
  // display the keyframe
  public void display() {
    colorMode(RGB, 255);
    if (selected)
      fill (255, 255, 255);
    else
      fill (r, g, b);
    ellipse(x, y, w, w);
  }
//  void change(int time0, int value0) {
//     time = time0;
//     value = value0;
//  }
//  
//  void move(float x0, int y0) {
//     x = x0;
//     y = y0;
//  }
//
//  void changeAndMove(int time0, int value0, float x0, float y0) {
//    time = time0;
//    value = value0;
//    x = x0;
//    y = y0;
//  }
  
}
  static public void main(String[] passedArgs) {
    String[] appletArgs = new String[] { "LightShow" };
    if (passedArgs != null) {
      PApplet.main(concat(appletArgs, passedArgs));
    } else {
      PApplet.main(appletArgs);
    }
  }
}
