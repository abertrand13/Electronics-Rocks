import point2line.*;

Vect2 velocity = new Vect2( 1, 0 );

println( degrees( velocity.angle() ) );  // prints "0.0"

Vect2 rightVelocity = velocity.rotatedRight();

println( degrees( rightVelocity.angle() ) );  // prints "90.0"
