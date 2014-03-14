import point2line.*;

Vect2 position1 = new Vect2( 100, 100 );
Vect2 position2 = new Vect2( 200, 200 );

float distance = Vect2.distance( position1, position2 );

println( distance ); // prints "141.42136"
