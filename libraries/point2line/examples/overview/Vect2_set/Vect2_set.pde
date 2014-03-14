import point2line.*;

Vect2 velocity = new Vect2( 1, 0 );

println( velocity );  // prints "Vect2( 1.0, 0.0 )"

velocity.set( 5, 5 );

println( velocity );  // prints "Vect2( 5.0, 5.0 )"

Vect2 newVelocity = new Vect2( 10, 10 );
velocity.set( newVelocity );

println( velocity );  // prints "Vect2( 10.0, 10.0 )"
