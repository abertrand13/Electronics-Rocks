import point2line.*;

Vect2 velocity = new Vect2( 1, 0 );

println( degrees( velocity.angle() ) );  // prints "0.0"

Vect2 leftVelocity = velocity.rotatedLeft();

println( degrees( leftVelocity.angle() ) );  // prints "270.0"
