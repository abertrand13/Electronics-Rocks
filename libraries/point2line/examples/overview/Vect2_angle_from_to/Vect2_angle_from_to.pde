import point2line.Vect2;

Vect2 velocity1 = new Vect2( 1, 0 );
Vect2 velocity2 = new Vect2( 0, -1 );

float angle = Vect2.angleFromTo( velocity1, velocity2 );

println( degrees( angle ) ); // prints -90.0
