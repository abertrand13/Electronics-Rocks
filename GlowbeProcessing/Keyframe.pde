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
  
  
  Keyframe(int time0, int value0, float x0, float y0, color c) {
    time = time0;
    value = value0;
    x = x0;
    y = y0;
    w = 5;
    r = red(c);
    g = green(c);
    b = blue(c);
    beingDragged = false;
    selected = false;
  }
  

  void select() {
    beingDragged = true;
    selected = true;
  }
  
  void stopDragging() {
    beingDragged = false;
    println(value);
  }
  
  void deselect() {
    beingDragged = false;
    selected = false;  
  }
  

  
  // display the keyframe
  void display() {
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
