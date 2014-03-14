import point2line.*;

Vect2 velocity = new Vect2( 10, 10 );

println( velocity.magnitude() ); // prints "14.142136"

Vect2 clippedVelocity = velocity.clipped( 20 );

println( clippedVelocity.magnitude() ); // prints "14.142136"

clippedVelocity = velocity.clipped( 5 );

println( clippedVelocity.magnitude() ); // prints "5.0"
