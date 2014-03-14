import point2line.*;

Vect2 position = new Vect2( 0, 0 );
Vect2 velocity = new Vect2( 1, 1 );

Vect2 newPosition = position.added( velocity );

println( newPosition ); // prints "Vect2( 1.0, 1.0 )"
