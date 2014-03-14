class Particle
{
  Vect2 position;
  Vect2 velocity;
  
  
  Particle(){
    position = new Vect2( random( width ), random( height ) );
    velocity = new Vect2();
  }


  void update(){
    // move //
    position.add( velocity );

    // stay in scene //
    reboundEdges();
  }
  
  
  void addForce( Vect2 force ){
    velocity.add( force );
  }


  void damp( float dampMult ){
    velocity.scale( 1-dampMult );
  }


  void display(){
    noStroke();
    fill( 0 );
    ellipse( position.x, position.y, DIAMETER, DIAMETER );
    fill( 0, 100 );
    ellipse( position.x, position.y, DIAMETER+PADDING*2, DIAMETER+PADDING*2 );
  }


  void reboundEdges(){
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

