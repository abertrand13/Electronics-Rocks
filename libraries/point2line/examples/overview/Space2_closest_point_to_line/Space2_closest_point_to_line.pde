import point2line.*;

Vect2 linePoint1 = new Vect2( 0, 0 );
Vect2 linePoint2 = new Vect2( 100, 100 );
Vect2[] testPoints = new Vect2[ 100 ];
for( int i=0; i<testPoints.length; i++ ){
  testPoints[ i ] = new Vect2( random( 100 ), random( 100 ) );
}

int closetPointIndex = Space2.closestPointToLine( testPoints, linePoint1, linePoint2 );

println( testPoints[ closetPointIndex ] );
