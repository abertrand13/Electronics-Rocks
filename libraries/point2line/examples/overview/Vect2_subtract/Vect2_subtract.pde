import point2line.*;

Vect2 position1 = new Vect2( 50, 50 );
Vect2 position2 = new Vect2( 200, 200 );

// calculate a vector pointing from position1 towards position2 //
Vect2 direction = position2.copy();
direction.subtract( position1 );

println( direction ); // prints "Vect2( 150.0, 150.0 )"
