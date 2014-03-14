import point2line.*;

Vect2 position = new Vect2( 0, 0 );
Vect2 velocity = new Vect2( 1, 1 );

position.add( velocity );

println( position ); // prints "Vect2( 1.0, 1.0 )"

