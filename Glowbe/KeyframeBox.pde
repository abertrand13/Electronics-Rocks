// A box and its keyframes

class KeyframeBox { 
  int x; // x coordinate of top-left corner
  int y; // y coordinate of this corner
  int boxWidth;
  int boxHeight;
  color keyframeColor;
  ArrayList<Keyframe> keyframes; // stores keyframes in ascending x order

  KeyframeBox(int x0, int y0, int width0, int height0, int showLength, color kfColor) {
    x = x0;
    y = y0;
    boxWidth = width0;
    boxHeight = height0;
    keyframeColor = kfColor; 
    keyframes = new ArrayList<Keyframe>();
    keyframes.add(new Keyframe(0, 255, x, y, kfColor));
    keyframes.add(new Keyframe(showLength, 255, x+boxWidth, y, kfColor));
  }
}
