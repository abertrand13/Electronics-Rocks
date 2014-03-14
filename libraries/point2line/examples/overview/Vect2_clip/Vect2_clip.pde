import point2line.*;

Vect2 velocity = new Vect2( 10, 10 );

println( velocity.magnitude() ); // prints "14.142136"

velocity.clip( 20 );

println( velocity.magnitude() ); // prints "14.142136"

velocity.clip( 5 );

println( velocity.magnitude() ); // prints "5.0"
