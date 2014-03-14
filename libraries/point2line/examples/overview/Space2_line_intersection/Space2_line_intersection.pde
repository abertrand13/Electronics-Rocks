import point2line.*;

Vect2[] line1;
Vect2[] line2;

void setup()
{
  size( 200, 200 );
  smooth();
  fill( 255, 0, 0 );
  stroke( 255 );

  line1 = new Vect2[ 2 ];
  line1[ 0 ] = new Vect2( 0, height* 0.4 );
  line1[ 1 ] = new Vect2( width, height* 0.9 );
  
  line2 = new Vect2[ 2 ];
  line2[ 0 ] = new Vect2( width*0.5, 0 );
  line2[ 1 ] = new Vect2(); // will be updateed by the mouse
}


void draw()
{
  line2[ 1 ] = new Vect2( mouseX, mouseY );
  
  Vect2 intersectionPoint = Space2.lineIntersection( line1[ 0 ], line1[ 1 ], line2[ 0 ], line2[ 1 ] );
  
  // display //
  background( 0 );
  if( intersectionPoint != null ){
    ellipse( intersectionPoint.x, intersectionPoint.y, 10, 10 );
  }
  line( line1[ 0 ].x, line1[ 0 ].y, line1[ 1 ].x, line1[ 1 ].y );
  line( line2[ 0 ].x, line2[ 0 ].y, line2[ 1 ].x, line2[ 1 ].y );
}
