import point2line.*;

Vect2 position1 = new Vect2( 0, 0 );
Vect2 position2 = new Vect2( 100, 100 );

Vect2 halfway = Vect2.lerp( position1, position2, 0.5 );

println( halfway );  // prints "Vect2( 50.0, 50.0 )"
