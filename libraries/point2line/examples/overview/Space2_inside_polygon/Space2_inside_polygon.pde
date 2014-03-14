import point2line.*;

Vect2[] vertices;

void setup(){
  size( 200, 200 );
  smooth();
  
  vertices = new Vect2[ 4 ];
  vertices[ 0 ] = new Vect2( 0, height*0.5 );
  vertices[ 1 ] = new Vect2( width, 0 );
  vertices[ 2 ] = new Vect2( width*0.5, height*0.5 );
  vertices[ 3 ] = new Vect2( width, height );
}


void draw()
{
  Vect2 mouse = new Vect2( mouseX, mouseY );
  
  boolean isInside = Space2.insidePolygon( mouse, vertices );
  
  // display //
  background( 0 );
  if( isInside ) fill( 0, 255, 0 );
  else fill( 255, 0, 0 );
  beginShape();
  for( int i=0; i<vertices.length; i++ ) vertex( vertices[ i ].x, vertices[ i ].y );
  endShape( CLOSE );
}
