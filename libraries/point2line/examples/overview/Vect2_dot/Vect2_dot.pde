import point2line.*;

Vect2 right  = new Vect2( radians( 0 ) );
Vect2 down   = new Vect2( radians( 90 ) );
Vect2 left   = new Vect2( radians( 180 ) );
Vect2 up     = new Vect2( radians( 270 ) );

Vect2 almostRight = new Vect2( radians( 10 ) );

println( Vect2.dot( almostRight, right ) ); // prints "0.9848077"
println( Vect2.dot( almostRight, down ) );  // prints "0.17364813"
println( Vect2.dot( almostRight, left ) );  // prints "-0.9848077"
println( Vect2.dot( almostRight, up ) );    // prints "-0.17364816"
