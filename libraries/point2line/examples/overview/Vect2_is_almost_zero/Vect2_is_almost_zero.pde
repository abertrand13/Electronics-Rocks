/**
*  using isAlmostZero() to test when to end an easing
*/

import point2line.*;

boolean isEaseing;
Vect2 position;

void setup(){
  size( 200, 200 );
  smooth();
  startEase();
}

void startEase(){
  position = new Vect2( random( -width*0.5, width*0.5), random( -height*0.5, height*0.5) );
  isEaseing = true;
}

void draw()
{
  if( isEaseing ){
    position.scale( 0.95 );
    isEaseing = !position.isAlmostZero();
  }
  
  // display //
  background( 0 );
  translate( width*0.5, height*0.5 );
  if( isEaseing ) fill( 0, 255, 0 );
  else fill( 255, 0, 0 );
  ellipse( position.x, position.y, 5, 5 );
}

void mousePressed(){
  startEase();
}
