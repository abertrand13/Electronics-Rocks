import point2line.*;

Vect2 position1 = new Vect2( 0, 0 );
Vect2 position2 = new Vect2( 100, 0 );

Vect2 midpoint = Vect2.midpoint( position1, position2 );

println( midpoint ); // prints Vect2( 50.0, 0.0 )
