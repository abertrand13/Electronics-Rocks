import point2line.Vect2;

Vect2 position1 = new Vect2( 50, 50 );
Vect2 position2 = new Vect2( 200, 200 );

// rotate position2 90 degrees around position1 //
Vect2 rotated = position2.rotatedAround( position1, radians( 90 ) );
