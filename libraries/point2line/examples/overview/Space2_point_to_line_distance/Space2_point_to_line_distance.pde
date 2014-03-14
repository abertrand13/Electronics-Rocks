import point2line.*;

Vect2 pointA;
Vect2 pointB;

void setup()
{
  size( 200, 200 );
  smooth();
  stroke( 255 );

  pointA = new Vect2( 0, height* 0.4 );
  pointB = new Vect2( width, height* 0.9 );
}


void draw()
{
  Vect2 mouse = new Vect2( mouseX, mouseY );
  
  float distance = Space2.pointToLineDistance( mouse, pointA, pointB );
  
  println( "distance: " + distance );
  
  // display //
  background( 0 );
  if( abs( distance ) < 10 ) stroke( 0, 255, 0 );
  else if( distance > 10 ) stroke( 255, 0, 0 );
  else if( distance < -10 ) stroke( 0, 0, 255 );
  line( pointA.x, pointA.y, pointB.x, pointB.y );
}
