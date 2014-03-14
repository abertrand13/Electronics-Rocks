import point2line.*;

Vect2 velocity = new Vect2( 0, 1 );

println( degrees( velocity.angle() ) );  // prints "90.0"

Vect2 rotatedVelocity = velocity.rotated( radians( 90 ) );

println( degrees( rotatedVelocity.angle() ) );  // prints "180.0"
