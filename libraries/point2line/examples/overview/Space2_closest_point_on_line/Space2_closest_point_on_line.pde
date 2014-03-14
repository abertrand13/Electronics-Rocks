import point2line.*;

Vect2 linePoint1 = new Vect2( 0, 0 );
Vect2 linePoint2 = new Vect2( 10, 10 );
Vect2 testPoint = new Vect2( 100, 0 );

Vect2 closestPoint = Space2.closestPointOnLine( testPoint, linePoint1, linePoint2 );

println( closestPoint ); // prints Vect2( 50.0, 50.0 )
