import point2line.*;

Vect2 velocity1 = new Vect2( 1, -1 );
Vect2 velocity2 = new Vect2( 1, 1 );

println( velocity1.equals( velocity2 ) ); // prints "false"
println( velocity2.equals( velocity1 ) ); // prints "false"

velocity1.y = 1;

println( velocity1.equals( velocity2 ) ); // prints "true"
println( velocity2.equals( velocity1 ) ); // prints "true"
