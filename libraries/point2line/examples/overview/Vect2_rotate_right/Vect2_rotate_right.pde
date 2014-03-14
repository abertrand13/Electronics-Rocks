import point2line.*;

Vect2 velocity = new Vect2( 1, 0 );

println( degrees( velocity.angle() ) );  // prints "0.0"

velocity.rotateRight();

println( degrees( velocity.angle() ) );  // prints "90.0"

