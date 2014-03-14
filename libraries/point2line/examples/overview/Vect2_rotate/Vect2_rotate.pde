import point2line.*;

Vect2 velocity = new Vect2( 0, 1 );

println( degrees( velocity.angle() ) );  // prints "90.0"

velocity.rotate( radians( 90 ) );

println( degrees( velocity.angle() ) );  // prints "180.0"
