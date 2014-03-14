import point2line.*;

Vect2 left = new Vect2( radians( 180 ) );

Vect2 right = new Vect2( 1, 0 );

Vect2 up = new Vect2( left.rotatedRight() );

println( degrees( left.angle() ) );  // prints "180.0"
println( degrees( right.angle() ) ); // prints "0.0"
println( degrees( up.angle() ) );    // prints "270.0"

println( left.magnitude() );  // prints "1.0"
println( right.magnitude() ); // prints "1.0"
println( up.magnitude() );    // prints "1.0"
