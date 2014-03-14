/**
*  Living Net
*  Example sketch for the point2line library
*  Carl Emil Carlsen
*  2009
*/

import point2line.*;


final int COUNT = 200;
final float MAX_BUBBLE_FORCE = 0.6;
final float MAX_MOUSE_FORCE = 0.5f;
final float MOUSE_RADIUS = 150;
final float DRAG_MULT = 0.02f;
final float PADDING = 7;
final float RADIUS = 14;
final float DIAMETER = RADIUS*2;
final float COLLISION_DISTANCE = DIAMETER + PADDING;
final color burnedColor = color( 255, 0, 0 );
final color coolColor = color( 0 );

boolean isTesting;

Particle[] particles;


void setup(){
  size( 600, 300 );
  smooth();
  background( 255 );

  particles = new Particle[ COUNT ];
  for (int i = 0; i < particles.length; i++) particles[ i ] = new Particle();
}


void draw(){
  
  if( !isTesting ){ // fade //
    noStroke();
    fill( 255, 30 );
    rect( 0, 0, width, height );
  } else { // clear screen //
    background( 255 );
  }
  

  // add bubble repulsion force //
  noFill();
  for (int i = 0; i < particles.length; i++) {
    for (int j = 0; j < particles.length; j++) {

      // make sure we only calculate for the same two particles once //
      if( i != j && i<j ){ 

        // calculate a vector pointing from paricle 'i' towards paricle 'j' //
        Vect2 towardsI = particles[ j ].position.subtracted( particles[ i ].position );

        // check for collision //
        if( towardsI.isLessThan( COLLISION_DISTANCE ) ){
          float distance  = towardsI.magnitude();
          
          // create a force towards paricle 'j' with the magnitude of 1 //
          Vect2 force = towardsI.divided( distance );

          // scale the force to maximum as they get closer //
          float forceInterpolator = 1 - ( distance / COLLISION_DISTANCE );
          force.scale( forceInterpolator * MAX_BUBBLE_FORCE );
          
          // add force to paricle 'j' //
          particles[ j ].addForce( force );

          // flip the force and add it to paricle 'i' //
          force.flip();
          particles[ i ].addForce( force );
          
          // display collision line //
          if( !isTesting ){
            stroke( lerpColor( coolColor, burnedColor, forceInterpolator ), 100 );
            line( particles[ i ].position.x, particles[ i ].position.y, particles[ j ].position.x, particles[ j ].position.y );
          }
        }
      }
    }
  }

  // save mouse position and speed into a vector //
  Vect2 mouse = new Vect2( mouseX, mouseY );
  Vect2 mouseSpeed = new Vect2( mouseX-pmouseX, mouseY-pmouseY );
  
  // add other forces, update and display //
  for (int i = 0; i < particles.length; i++) {
    
    // add mouse repulsion force //
    if( mouseSpeed.isGreaterThan( 0.5 ) ){
      Vect2 towardsBubble = particles[ i ].position.subtracted( mouse );
      if( towardsBubble.isLessThan( MOUSE_RADIUS ) ){
        float distance = towardsBubble.magnitude();
        float forceInterpolator = 1 - ( distance / MOUSE_RADIUS );
        Vect2 force = towardsBubble.normalized().scaled( forceInterpolator * MAX_MOUSE_FORCE );
        particles[ i ].addForce( force );
      }
    }

    // add damping //
    particles[ i ].damp( DRAG_MULT );

    // update //
    particles[ i ].update();
    
    // display //
    if( isTesting ) particles[ i ].display();
  }

}



void keyPressed(){
  if( key == ' ' ) isTesting = !isTesting;
}

