import point2line.*;

Vect2 velocity = new Vect2( 10, 10 );

Vect2 newVelocity = velocity.scaled( 2 );

print( newVelocity ); // prints "Vect2( 20.0, 20.0 )"
