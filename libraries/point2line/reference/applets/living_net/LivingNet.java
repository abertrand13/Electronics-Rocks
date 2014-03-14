import processing.core.*; 
import processing.xml.*; 

import point2line.*; 

import java.applet.*; 
import java.awt.*; 
import java.awt.image.*; 
import java.awt.event.*; 
import java.io.*; 
import java.net.*; 
import java.text.*; 
import java.util.*; 
import java.util.zip.*; 
import java.util.regex.*; 

public class LivingNet extends PApplet {

/**
*  Living Net
*  Example sketch for the point2line library
*  Carl Emil Carlsen
*  2009
*/




final int COUNT = 200;
final float MAX_BUBBLE_FORCE = 0.6f;
final float MAX_MOUSE_FORCE = 0.5f;
final float MOUSE_RADIUS = 150;
final float DRAG_MULT = 0.02f;
final float PADDING = 7;
final float RADIUS = 14;
final float DIAMETER = RADIUS*2;
final float COLLISION_DISTANCE = DIAMETER + PADDING;
final int burnedColor = color( 255, 0, 0 );
final int coolColor = color( 0 );

boolean isTesting;

Particle[] particles;


public void setup(){
  size( 600, 300 );
  smooth();
  background( 255 );

  particles = new Particle[ COUNT ];
  for (int i = 0; i < particles.length; i++) particles[ i ] = new Particle();
}


public void draw(){
  
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
    if( mouseSpeed.isGreaterThan( 0.5f ) ){
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



public void keyPressed(){
  if( key == ' ' ) isTesting = !isTesting;
}

class Particle
{
  Vect2 position;
  Vect2 velocity;
  
  
  Particle(){
    position = new Vect2( random( width ), random( height ) );
    velocity = new Vect2();
  }


  public void update(){
    // move //
    position.add( velocity );

    // stay in scene //
    reboundEdges();
  }
  
  
  public void addForce( Vect2 force ){
    velocity.add( force );
  }


  public void damp( float dampMult ){
    velocity.scale( 1-dampMult );
  }


  public void display(){
    noStroke();
    fill( 0 );
    ellipse( position.x, position.y, DIAMETER, DIAMETER );
    fill( 0, 100 );
    ellipse( position.x, position.y, DIAMETER+PADDING*2, DIAMETER+PADDING*2 );
  }


  public void reboundEdges(){
    if( position.x < 0 ){
      position.x = 0;
      velocity.x *=-1;
    } else if( position.x > width-1 ){
      position.x = width-1;
      velocity.x *=-1;
    }
    if( position.y < 0 ){
      position.y = 0;
      velocity.y *=-1;
    } else if( position.y > height-1 ){
      position.y = height-1;
      velocity.y *=-1;
    }
  }

}


  static public void main(String args[]) {
    PApplet.main(new String[] { "--bgcolor=#c0c0c0", "LivingNet" });
  }
}
