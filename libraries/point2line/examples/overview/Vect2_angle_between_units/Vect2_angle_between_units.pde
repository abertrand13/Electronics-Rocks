import point2line.*;

Vect2 left = new Vect2( -1, 0 );
Vect2 up   = new Vect2( 0, -1 );

println( degrees( left.angle() ) );  // prints "180.0"
println( degrees( up.angle() ) );  // prints "270.0"

println( degrees( Vect2.angleBetweenUnits( left, up ) ) );  // prints "90.0"
