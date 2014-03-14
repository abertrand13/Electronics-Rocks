/*
 * Lets you make a light show!
 * Needs more commenting + explanation...
 */
 
import controlP5.*;

ControlP5 cp5;
Textlabel instructionsLabel;

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

void setup() {  
  size(360, 640);
  colorMode(RGB, 255);
  noStroke();
  background(255, 255, 255);
  keyframeBoxes =  new KeyframeBox[3];
  keyframeBoxes[0] = new KeyframeBox(firstBoxX, firstBoxY, boxWidth, boxHeight, showLength, color(255, 0, 0));
  keyframeBoxes[1] = new KeyframeBox(firstBoxX, firstBoxY+boxHeight+10, boxWidth, boxHeight, showLength, color(0, 255, 0));
  keyframeBoxes[2] = new KeyframeBox(firstBoxX, firstBoxY+2*boxHeight+20, boxWidth, boxHeight, showLength, color(0, 0, 255)); 
                   
  cp5 = new ControlP5(this);
  instructionsLabel = cp5.addTextlabel("label")
    .setText("LIGHT SHOW (currently only available in red)\n*Click anywhere in the black box to create a new\nkeyframe\n*Keyframes can be clicked and dragged to change\nthe animation\n*Delete or Backspace will erase the highlighted\nkeyframe\n*Click PLAY! to test the show")
    .setPosition(10, firstBoxY-240)
    .setColorValue(0xfff00000)
    .setFont(createFont("Georgia",15))
    ;
    
  cp5.addButton("playAnimation")
    .setValue(0)
    .setCaptionLabel("Play!")
    .setPosition(firstBoxX, firstBoxY-90)
    .setSize(boxWidth, 25);
    
  cp5.addButton("stopAnimation")
    .setValue(0)
    .setCaptionLabel("Stop")
    .setPosition(firstBoxX, firstBoxY-60)
    .setSize(boxWidth, 25);
    
  cp5.addSlider("slider")
    .setCaptionLabel("Animation length")
    .setColorCaptionLabel(0)
    .setPosition(firstBoxX, firstBoxY-30)
    .setSize(boxWidth/2, 25)
    .setValue(200)
    .setRange(10,1000)
    ;
}

void draw() {
  // Coloring the box
  if (animating)
    animate();
    
  fill(10, 10, 10);
  for (int i=0; i<3; i++) {
    KeyframeBox kfBox = keyframeBoxes[i];
    rect(kfBox.x, kfBox.y, kfBox.boxWidth, kfBox.boxHeight);
  }
  
  // time ticker
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

// called when "Play!" button pressed
public void playAnimation(int value) {
  animating = true;
  frame = 0;
  lastKeyframeIndices[0] = 0;
  lastKeyframeIndices[1] = 0;
  lastKeyframeIndices[2] = 0;
}

// called when "Stop" button pressed
public void stopAnimation(int value) {
  animating = false;
  frame = 0;
}

void slider(int theDuration) {
  showLength = theDuration;
  animating = false; // to prevent outOfBoundsExceptions, for now
  for (int b=0; b < 3; b++) {
    for (int i=0; i < keyframeBoxes[b].keyframes.size(); i++) {
       Keyframe kf = keyframeBoxes[b].keyframes.get(i);
       kf.time = xToTime(kf.x, keyframeBoxes[b]);
    }
  }
}

void animate() {
  int rgb[] = new int[3];
  for (int b = 0; b < 3; b++) {
    Keyframe lastKeyframe = keyframeBoxes[b].keyframes.get(lastKeyframeIndices[b]);
    Keyframe nextKeyframe = keyframeBoxes[b].keyframes.get(lastKeyframeIndices[b]+1);
    int duration = nextKeyframe.time-lastKeyframe.time;
    int timeSinceLast = frame - lastKeyframe.time;
    float progress = ((float)timeSinceLast)/duration;
    rgb[b] = (int) lerp(lastKeyframe.value, nextKeyframe.value, progress); 
    
    while (lastKeyframeIndices[b] < keyframeBoxes[b].keyframes.size() - 1  && frame > keyframeBoxes[b].keyframes.get(lastKeyframeIndices[b]+1).time) {
      lastKeyframeIndices[b]++;
    }
  }
  
  background(rgb[0], rgb[1], rgb[2]);
  frame++;
  if (frame >= showLength) {
    frame = 0;
    lastKeyframeIndices[0] = 0;
    lastKeyframeIndices[1] = 0;
    lastKeyframeIndices[2] = 0;
  }
}

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
