import point2line.*;

Vect2 velocity = new Vect2( 0, 1 );

println( degrees( velocity.angle() ) );  // prints "90.0"

velocity.flip();

println( degrees( velocity.angle() ) );  // prints "270.0"
