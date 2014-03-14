import point2line.*;

Vect2 right = new Vect2( 1, 0 );
Vect2 down  = new Vect2( 0, 1 );
Vect2 left  = new Vect2( -1, 0 );
Vect2 up    = new Vect2( 0, -1 );

println( degrees( right.angle() ) ); // prints "0.0"
println( degrees( down.angle() ) );  // prints "90.0"
println( degrees( left.angle() ) );  // prints "180.0"
println( degrees( up.angle() ) );    // prints "270.0"

