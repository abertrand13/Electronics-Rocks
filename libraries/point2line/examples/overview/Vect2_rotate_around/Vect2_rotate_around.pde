import point2line.Vect2;

Vect2 position1 = new Vect2( 50, 50 );
Vect2 position2 = new Vect2( 200, 200 );

// rotate position2 90 degrees around position1 //
position2.rotateAround( position1, radians( 90 ) );
