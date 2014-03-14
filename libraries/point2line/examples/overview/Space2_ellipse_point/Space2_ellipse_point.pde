import point2line.*;

float angle = 30;
float width = 100;
float height = 50;
Vect2 ellipsePoint = Space2.ellipsePoint( radians( angle ), width , height );

println( ellipsePoint ); // prints "Vect2( 32.732685, 18.898224 )"
