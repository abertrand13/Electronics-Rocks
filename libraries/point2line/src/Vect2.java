package point2line;


/**
 * Vect2 is a class for storing and manipulating vectors and positions in 2D.<br/>
 * <br>
 * <p><b>WHY ANOTHER VECTOR CLASS?</b></p>
 * There are a great number of open source vector classes available that does 
 * practically the same, so choosing a vector class for consistent use is mostly 
 * about taste. Processing already has the build-in PVector class by 
 * <a href="http://shiffman.net/">Daniel Shiffman</a>.
 * The main arguments for using Vect2 are:<br/>
 * <ul>
 * 		<li>Additional commonly used methods</li>
 * 		<li>Optimized convenience methods</li>
 * 		<li>Method chaining</li>
 * </ul>
 * Also, PVector is made for both 2D and 3D purposes which makes Vect2 slightly faster 
 * when working in 2D. I have chosen meaningful naming in favor of short keywords, 
 * making it more attractive for non-programmers.<br>
 * <br/>
 * 
 * <p><b>ABOUT METHOD CHAINING</b></p>
 * The methods that are named in past tense (<code>eg. added(), flipped()</code>) all return 
 * a copy of the vector, leaving the original untouched. These are all convenience methods
 * that allow to make series of methods calls in one line. Example:<br/>
 * <code>Vect2 force = pos1.subtracted(pos2).normalized.scaled(maxForce);</code><br/>
 * <br/>
 * 
 * <p><b>LICENCE</b></p>
 * Vect2 is part of the point2line library. The library is free software; you can redistribute it 
 * and/or modify it for any desirable purpose. It is distributed in the hope that 
 * it will be useful, but without any warranty.<br>
 * <br/>
 * 
 * <p><b>AUTHORS</b></p>
 * Vect2 is maintained and updated by 
 * <a href="http://sixthsensor.dk/">Carl Emil Carlsen</a> 
 * in dialog with 
 * <a href="http://stimulacrum.com/">Daniel Hoeier Oehrgaard</a> 2005+ <br/>
 * <br/>
 * 
 * @author <a href="http://sixthsensor.dk/">Carl Emil Carlsen</a>
 * @author <a href="http://stimulacrum.com/">Daniel Hoeier Oehrgaard</a>
 * @usage Web & Application
 */


public class Vect2
{
	/** 
	 * The x component of the vector 
	 * @related y
	 * @example Vect2_x
	 */
	public float x;
	
	/** 
	 * The y component of the vector
	 * @related x
	 * @example Vect2_y
	 */
	public float y;
	
	// for internal calculations //
	private static final float TWO_PI = (float) ( Math.PI * 2f );
	
	
	
	//////////////////
	// CONSTRUCTORS //
	//////////////////
	
	
	public Vect2(){ setZero(); }

	/** @param x,y components of new vector (<code>float,float</code>) */
	public Vect2( float x, float y ){ set( x, y ); }

	/** @param vector to copy into new vector (<code>Vect2</code>) */
	public Vect2( Vect2 vector ){ set( vector ); }
	
	/**
	 * Alternative constructor to create a unit vector at a specific angle.
	 * @param  angle in radians to set the rotation of new unit vector (<code>float</code>)
	 */
	public Vect2( float angle )
	{
		x = (float) Math.cos( angle );
		y = (float) Math.sin( angle );
	}
	
	
	
	////////////////
	// PROPERTIES //
	////////////////
	
	
	/**
	 * Returns a boolean that is true if the vector is zero.
	 * In other words, it returns ( x==0 && y==0 ).
	 * 
	 * @shortdesc Returns <code>true</code> if the vector is zero.
	 * @return true if vector is zero (<code>boolean</code>)
	 * @related isAlmostZero ( )
	 * @example Vect2_is_zero
	 */
	public boolean isZero() {
		return (x == 0f && y == 0f);
	}
	
	
	/**
	 * Returns a boolean that is true if the vector is almost zero 
	 * ( +/- 0.001f ). This is useful when easing towards zero to 
	 * check if the target has been reached.
	 * 
	 * @shortdesc Returns <code>true</code> if the vector is almost zero.
	 * @return true if vector is almost zero (<code>boolean</code>)
	 * @related isZero ( )
	 * @example Vect2_is_almost_zero
	 */
	public boolean isAlmostZero(){
		return( Math.abs( x ) < 0.1f && Math.abs( y ) < 0.1f );
	}
	
	
	/**
	 * Returns a boolean that is true if the vector is longer than the 
	 * input magnitude. This function is about ten times faster than 
	 * a regular distance check, because it avoids using square root and 
	 * division.
	 * 
	 * @shortdesc Returns <code>true</code> if the vector is longer than a specific magnitude.
	 * @param magnitude to compare against (<code>float</code>)
	 * @param magnitude to compare against (<code>int</code>)
	 * @return true if the vector is longer than the given magnitude (<code>boolean</code>)
	 * @related isLessThan ( )
	 * @related isZero ( )
	 * @related isAlmostZero ( )
	 * @example Vect2_is_greater_than
	 */
	public boolean isGreaterThan( float magnitude ) {
		return( squareMagnitude() > magnitude * magnitude );
	}
	public boolean isGreaterThan( int magnitude ) {
		return( squareMagnitude() > magnitude * magnitude );
	}
	
	
	/**
	 * Returns a boolean that is true if the vector is shorter than the 
	 * input magnitude. This function is about ten times faster than 
	 * a regular distance check, because it avoids using square root and 
	 * division.
	 * 
	 * @shortdesc Returns <code>true</code> if the vector is shorter than a specific magnitude.
	 * @param magnitude to compare against (<code>float</code>)
	 * @param magnitude to compare against (<code>int</code>)
	 * @return true if the vector is shorter than the given magnitude (<code>boolean</code>)
	 * @related isGreaterThan ( )
	 * @related isZero ( )
	 * @related isAlmostZero ( )
	 * @example Vect2_is_less_than
	 */
	public boolean isLessThan( float magnitude ) {
		return( squareMagnitude() < magnitude * magnitude );
	}
	public boolean isLessThan( int magnitude ) {
		return( squareMagnitude() < magnitude * magnitude );
	}
	
	
	/**
	 * Returns a boolean that is true if the vector is equals an input vector.
	 * 
	 * @shortdesc Returns <code>true</code> if the vector is equals another vector
	 * @param vector to compare against (<code>Vect2</code>)
	 * @return true if the given vector is equal this vector (<code>boolean</code>)
	 * @example Vect2_equals
	 */
	public boolean equals( Vect2 vector ) {
		if( vector == null ) return false;
		return( this.x == vector.x && this.y == vector.y );
	}
	
	
	
	
	
	////////////////
	// BASIC MATH //
	////////////////
	
	
	/**
	 * Multiplies the vector by an input value and stores the result.
	 * 
	 * @shortdesc Multiplies the vector by an input value and stores the result.
	 * @param multiplier to scale this vector (<code>float</code>)
	 * @param multX,multY to scale this vector (<code>float,float</code>)
	 * @param multVect to scale this vector (<code>Vect2</code>)
	 * @related scaled ( )
	 * @related divide ( )
	 * @related add ( )
	 * @related subtract ( )
	 * @example Vect2_scale
	 */
	public void scale( float multiplier ){
		x *= multiplier;
		y *= multiplier;
	}
	public void scale( float multX, float multY ){
		x *= multX;
		y *= multY;
	}
	public void scale( Vect2 multVect ){
		x *= multVect.x;
		y *= multVect.y;
	}
	
	
	/**
	 * Multiplies the vector by an input value and returns the result without changing the original.
	 * 
	 * @shortdesc Multiplies the vector by an input value and returns the result.
	 * @param multiplier to scale this vector (<code>float</code>)
	 * @param multX,multY to scale this vector (<code>float,float</code>)
	 * @param multVector to scale this vector (<code>Vect2</code>)
	 * @return new vector containing the result of the multiplication (<code>Vect2</code>)
	 * @related scale ( )
	 * @related divided ( )
	 * @related added ( )
	 * @related subtracted ( )
	 * @example Vect2_scaled
	 */
	public Vect2 scaled( float multiplier ){
		Vect2 result = this.copy();
		result.x *= multiplier;
		result.y *= multiplier;
		return result;
	}
	public Vect2 scaled( float multX, float multY ){
		Vect2 result = this.copy();
		result.x *= multX;
		result.y *= multY;
		return result;
	}
	public Vect2 scaled( Vect2 multVector ){
		Vect2 result = this.copy();
		result.x *= multVector.x;
		result.y *= multVector.y;
		return result;
	}

	
	/**
	 * Divides the vector by a divider and stores the result.
	 * 
	 * @shortdesc Divides the vector by a divider and stores the result.
	 * @param divider to divide this vector (<code>float</code>)
	 * @related divided ( )
	 * @related scale ( )
	 * @related add ( )
	 * @related subtract ( )
	 * @example Vect2_divide
	 */
	public void divide( float divider ) {
		if( divider != 0 ){
			scale( 1 / divider );
		}
	}
	
	
	/**
	 * Divides the vector by a divider and returns the result without changing the original.
	 * 
	 * @shortdesc Divides the vector by a divider and returns the result.
	 * @param divider to divide this vector (<code>float</code>)
	 * @return new vector containing the result of the division (<code>Vect2</code>)
	 * @related divide ( )
	 * @related scaled ( )
	 * @related added ( )
	 * @related subtracted ( )
	 * @example Vect2_divided
	 */
	public Vect2 divided( float divider ) {
		if( divider != 0 ){
			Vect2 result = this.copy();
			result.scale( 1 / divider );
			return result;
		}
		return new Vect2( 0, 0 );
	}
	

	/**
	 * Adds an input vector to the vector and stores the result.
	 * 
	 * @shortdesc Adds an input vector to the vector and stores the result.
	 * @param x,y to be added to the vector (<code>float, float</code>)
	 * @param vector to be added to the vector (<code>Vect2</code>)
	 * @related added ( )
	 * @related subtract ( )
	 * @related scale ( )
	 * @related divide ( )
	 * @example Vect2_add
	 */
	public void add( float x, float y ){
		this.x += x;
		this.y += y;
	}
	public void add( Vect2 vector ) {
		this.x += vector.x;
		this.y += vector.y;
	}
	
	
	/**
	 * Returns the result of an input vector added to the vector without changing the originals.
	 * 
	 * @shortdesc Adds an input vector to the vector and returns the result.
	 * @param x,y to be added to the vector (<code>float, float</code>)
	 * @param vector to be added to the vector (<code>Vect2</code>)
	 * @return new vector containing the result of the addition (<code>Vect2</code>)
	 * @related add ( )
	 * @related subtracted ( )
	 * @related scaled ( )
	 * @related divided ( )
	 * @example Vect2_added
	 */
	public Vect2 added( float x, float y ){
		Vect2 result = this.copy();
		result.x += x;
		result.y += y;
		return result;
	}
	public Vect2 added( Vect2 vector ) {
		Vect2 result = this.copy();
		result.x += vector.x;
		result.y += vector.y;
		return result;
	}
	
	
	/** 
	 * Subtracts an input from the vector and stores the result.<br />
	 * 
	 * @shortdesc Subtracts an input from the vector and stores the result.
	 * @param x,y components to subtract from the vector (<code>float,float</code>)
	 * @param vector to subtract from the vector (<code>Vect2</code>)
	 * @related subtracted ( )
	 * @related add ( )
	 * @related scale ( )
	 * @related divide ( )
	 * @example Vect2_subtract
	 */
	public void subtract( float x, float y ){
		this.x -= x;
		this.y -= y;
	}
	public void subtract( Vect2 vector ){
		this.x -= vector.x;
		this.y -= vector.y;
	}
	
	
	/** 
	 * Subtracts and input vector from the vector and returns the result without changing the originals.
	 * 
	 * @shortdesc Subtracts and input vector from the vector and returns the result.
	 * @param x,y components to subtract from the vector (<code>float,float</code>)
	 * @param vector to subtract from the vector (<code>Vect2</code>)
	 * @return new vector containing the result of the subtraction (<code>Vect2</code>)
	 * @related subtract ( )
	 * @related added ( )
	 * @related scaled ( )
	 * @related divided ( )
	 * @example Vect2_subtracted
	 */
	public Vect2 subtracted( float x, float y ){
		Vect2 result = this.copy();
		result.x -= x;
		result.y -= y;
		return result;
	}
	public Vect2 subtracted( Vect2 vector ){
		Vect2 result = this.copy();
		result.x -= vector.x;
		result.y -= vector.y;
		return result;
	}
	
	
	
	///////////////
	// MAGNITUDE //
	///////////////
	
	
	/**
	 * Returns the magnitude of the vector, also referred to as the length.
	 * 
	 * @return magnitude of this vector (<code>float</code>)
	 * @related setMagnitude ( )
	 * @example Vect2_magnitude
	 */
	public float magnitude()
	{
		return (float) Math.sqrt( ( x * x ) + ( y * y ) );
	}
	
	
	/**
	 * Returns the square magnitude (square length) of the vector. <br>
	 * <br>
	 * The square root function which is used when calling magnitude() is horrendously slow, 
	 * so try to avoid calculating vector lengths whenever you can. A common problem in computer 
	 * graphics is to find the shortest vector in a list, in this case you only need to calculate 
	 * the square magnitude (x*x + y*y) for each of them and find the smallest value from that 
	 * (since the vector with the shortest length will also have the smallest squared length).
	 * 
	 * @shortdesc Returns the square magnitude of the vector.
	 * @return square magnitude of this vector (<code>float</code>)
	 * @related magnitude ( )
	 * @related setMagnitude ( )
	 * @example Vect2_square_magnitude
	 */
	public float squareMagnitude()
	{
		return x * x + y * y;
	}
	
	
	/**
	 * Normalizes the vector. Same as calling <code>setMagnitude( 1 )</code>.
	 * 
	 * @shortdesc Normalizes the vector.
	 * @related normalized ( )
	 * @related setMagnitude ( )
	 * @example Vect2_normalize
	 */
	public void normalize() {
		setMagnitude( 1 );
	}
	
	
	/**
	 * Returns a copy that has been normalized.
	 * 
	 * @shortdesc Returns a copy that has been normalized.
	 * @return new normalized copy of the vector (<code>Vect2</code>)
	 * @related normalize ( )
	 * @related setMagnitude ( )
	 * @example Vect2_normalized
	 */
	public Vect2 normalized() {
		Vect2 normalized = new Vect2( this.x, this.y );
		normalized.normalize();
		return normalized;
	}
	
	
	/**
	 * Sets the magnitude (length) of the vector.
	 * 
	 * @param magnitude (<code>int</code>)
	 * @param magnitude (<code>float</code>)
	 * @related normalize ( )
	 * @example Vect2_set_magnitude
	 */
	public void setMagnitude( int magnitude ) {
		if( !isZero() ) {
			float mult = ( (float) magnitude ) / magnitude();
			scale(mult);
		}
	}
	public void setMagnitude( float magnitude ) {
		if( !this.isZero() ){
			float mult = magnitude / magnitude();
			this.scale( mult );
		}
	}
	
	
	/** 
	 * Clips the magnitude of the vector. In other words, if the
	 * magnitude exceeds the input value, it is set to that value.
	 * 
	 * @shortdesc Clips the magnitude of the vector.
	 * @param maximum magnitude (<code>float</code>)
	 * @related clipped ( )
	 * @related setMagnitude ( )
	 * @example Vect2_clip
	 */
	public void clip( float maximum ) {
		if( magnitude() > maximum ) setMagnitude( maximum );
	}
	
	
	/** 
	 * Returns a copy of the vector that has been clipped.
	 * 
	 * @param maximum magnitude (<code>float</code>)
	 * @related clip ( )
	 * @related setMagnitude ( )
	 * @example Vect2_clipped
	 */
	public Vect2 clipped( float maximum ) {
		Vect2 result = this.copy();
		if( result.magnitude() > maximum ) result.setMagnitude( maximum );
		return result;
	}
	
	
	
	
	//////////////
	// ROTATION //
	//////////////
	
	
	/**
	 * Returns the angle of the vector in radians. All rotations are based on 
	 * the unit circle where angle 0 and PI*2 is 3 o'clock.
	 * 
	 * @shortdesc Returns the angle of the vector.
	 * @return angle of this vector in radians (<code>float</code>)
	 * @example Vect2_angle
	 */
	public float angle()
	{
		float a = (float) Math.atan2( y, x );
		if( a < 0 ) {
			a += TWO_PI;
			if( a == TWO_PI ) a = 0;
			return a;
		} else {
			return a;
		}
	}
	
	
	/**
	 * Rotates the vector clockwise by a delta angle 
	 * (turning left from the vectors forward direction).
	 * 
	 * @shortdesc Rotates the vector clockwise by a a delta angle.
	 * @param deltaAngle in radians (<code>float</code>)
	 * @related rotated ( )
	 * @related setRotation ( )
	 * @example Vect2_rotate
	 */
	public void rotate( float deltaAngle ){
		float x = this.x;
		float y = this.y;
		this.x = (float) ( x * Math.cos( deltaAngle ) - y * Math.sin( deltaAngle ) );
		this.y = (float) ( x * Math.sin( deltaAngle ) + y * Math.cos( deltaAngle ) );
	}
	
	
	/**
	 * Returns a copy that is rotated clockwise from the vector by a specified angle
	 * (turning left from the vectors forward direction).
	 * 
	 * @shortdesc Returns copy that has been rotated clockwise by a delta angle.
	 * @param angle in radians (<code>float</code>)
	 * @return a copy that has been rotated (<code>Vect2</code>)
	 * @related rotate ( )
	 * @related setRotation ( )
	 * @example Vect2_rotated
	 */
	public Vect2 rotated( float deltaAngle ){
		Vect2 result = this.copy();
		result.rotate( deltaAngle );
		return result;
	}
	
	
	/**
	 * Sets the rotation of the vector. All rotations are based on 
	 * the unit circle where angle 0 and PI*2 is 3 o'clock. It is slightly
	 * faster to call rotate() by a delta angle.
	 * 
	 * @shortdesc Sets the rotation of the vector.
	 * @param angle in radians (<code>float</code>)
	 * @related rotate ( )
	 * @example Vect2_set_rotation
	 */
	public void setRotation( float angle )
	{
		float len = this.magnitude();
		set( len, 0 );
		rotate( angle );
	}
	
	
	/**
	 * Rotates the vector 90 degrees right. The magnitude is left unchanged.
	 * This function is much faster than rotate() or setRotation().
	 * 
	 * @shortdesc Rotates the vector 90 degrees right.
	 * @related rotateLeft ( )
	 * @related rotatedLeft ( )
	 * @related rotatedRight ( )
	 * @related rotated ( )
	 * @related rotate ( )
	 * @related setRotation ( )
	 * @example Vect2_rotate_right
	 */
	public void rotateRight(){
		float newX = y;
		y = x;
		x = -newX;
	}
	
	
	/**
	 * Returns a copy that has been rotated 90 degrees clockwise from the vector's forward angle.
	 * The magnitude is unchanged.
	 * This function is much faster than rotate() or setRotation().
	 * 
	 * @shortdesc Returns a copy that has been rotated 90 degrees clockwise.
	 * @return	a copy that has been rotated 90 degrees clockwise (<code>Vect2</code>)
	 * @related rotatedLeft ( )
	 * @related rotateLeft ( )
	 * @related rotateRight ( )
	 * @related rotated ( )
	 * @related rotate ( )
	 * @related setRotation ( )
	 * @example Vect2_rotated_right
	 */
	public Vect2 rotatedRight()
	{
		Vect2 v = new Vect2( -y, x );
        return v;
    }
	
	
	/**
	 * Rotates the vector 90 degrees left. The magnitude is left unchanged.
	 * This function is much faster than rotate() or setRotation().
	 * 
	 * @shortdesc Rotates the vector 90 degrees left.
	 * @related rotateRight ( )
	 * @related rotatedLeft ( )
	 * @related rotatedRight ( )
	 * @related rotated ( )
	 * @related rotate ( )
	 * @related setRotation ( )
	 * @example Vect2_rotate_left
	 */
	public void rotateLeft(){
		float newX = y;
		y = -x;
		x = newX;
	}
	
	
	/**
	 * Returns a copy that has been rotated 90 degrees counter clockwise from the vector's forward angle.
	 * The magnitude is unchanged.
	 * This function is much faster than rotate() or setRotation().
	 * 
	 * @shortdesc Returns a copy that has been rotated 90 degrees counter clockwise.
	 * @return	a copy that has been rotated 90 degrees counter clockwise (<code>Vect2</code>)
	 * @related rotatedRight ( )
	 * @related rotateLeft ( )
	 * @related rotateRight ( )
	 * @related rotated ( )
	 * @related rotate ( )
	 * @related setRotation ( )
	 * @example Vect2_rotated_left
	 */
	public Vect2 rotatedLeft()
	{
		Vect2 v =  new Vect2( y, -x );
        return v;
    }
	
	
	/**
	 * Flips the vector 180 degrees from it's forward angle.
	 * 
	 * @shortdesc Flips the vector 180 degrees.
	 * @example Vect2_flip
	 */
	public void flip(){
		x *= -1;
		y *= -1;
	}
	
	
	/**
	 * Returns a copy that is flipped 180 degrees from the vector's forward angle.
	 * 
	 * @shortdesc Returns a copy that has been flipped.
	 * @return	new vector that is flipped 180 degrees from the vector (<code>Vect2</code>)
	 * @related flip ( )
	 * @related rotatedRight ( )
	 * @related rotatedLeft ( )
	 * @example Vect2_flipped
	 */
	public Vect2 flipped() {
		return new Vect2( -x, -y );
	}
	
	
	
	
	
	
	//////////////////
	// STATIC TOOLS //
	//////////////////
	
	
	/**
	 * Returns the euclidean distance between to positions stored in 
	 * two vectors.
	 * 
	 * @shortdesc Returns the  distance between to positions.
	 * @param vector1 containing a position (<code>Vect2</code>)
	 * @param vector2 containing a position (<code>Vect2</code>)
	 * @return distance between the two vectors (<code>float</code>)
	 * @example Vect2_distance
	 */
	public static float distance( Vect2 vector1, Vect2 vector2 ) {
		float dx = vector1.x - vector2.x;
		float dy = vector1.y - vector2.y;
		return (float) Math.sqrt( dx * dx + dy * dy );
	}
	
	
	/**
	 * Returns the dot product of this vector and another.
	 * The dot product value is in fact the cosine of the angle 
	 * between the two input vectors, multiplied by the lengths 
	 * of those vectors. So, you can easily calculate the cosine 
	 * of the angle by either, making sure that your two vectors 
	 * are both of length 1, or dividing the dot product by the 
	 * lengths.
	 * cos( theta ) = dot( v1, v2 ) / ( magnitude( v1 ) * magnitude( v2 ) )
	 * Values range from 1 to -1. If the two input vectors are 
	 * pointing in the same direction, then the return value will 
	 * be 1. If the two input vectors are pointing in opposite 
	 * directions, then the return value will be -1. If the two 
	 * input vectors are at right angles, then the return value 
	 * will be 0. So, in effect, it is telling you how similar 
	 * the two vectors are.
	 * 
	 * @shortdesc Returns the dot product of this vector and another.
	 * @param	vector1 (<code>Vect2</code>)
	 * @param	vector2 (<code>Vect2</code>)
	 * @return	dot product (<code>float</code>)
	 * @example Vect2_dot
	 */
	public static float dot( Vect2 vector1, Vect2 vector2 )
	{
        float dot = vector1.x * vector2.x + vector1.y * vector2.y;
        return dot;
    }
	
	
	/**
	 * Returns the linear interpolation between two vectors. In some vector
	 * classes this is referred to as blend().
	 * 
	 * @shortdesc Returns the linear interpolation between two vectors.
	 * @param begin the vector when interpolator is 0 (<code>Vect2</code>)
	 * @param end the vector when interpolator is 1 (<code>Vect2</code>)
	 * @param interpolator to lerp between the two vectors (<code>float</code>)
	 * @return interpolated vector (<code>Vect2</code>)
	 * @example Vect2_lerp
	 */
	public static Vect2 lerp( Vect2 begin, Vect2 end, float interpolator )
	{
		Vect2 v = new Vect2();
		v.x = ( end.x - begin.x ) * interpolator + begin.x;
		v.y = ( end.y - begin.y ) * interpolator + begin.y;
        return v;
    }
	
	
	/**
	 * Returns the angle between the forward angles of two vectors.<br>
	 * <br>
	 * The result is always positive. If you also need to know the rotational direction 
	 * (clockwise or counter-clockwise) you need to calculate the angle of each vector 
	 * and work it out from there.
	 * 
	 * @shortdesc Returns the angle between two vectors.
	 * @param vector1,vector2 (<code>Vect2,Vect2</code>)
	 * @return angle in radians, or NaN if vectors are parallel (<code>float</code>)
	 * @example Vect2_angle_between
	 */
	public static float angleBetween( Vect2 vector1, Vect2 vector2 ) {
		float theta = dot( vector1.normalized(), vector2.normalized() );
		return (float) Math.acos( theta );
	}
	
	
	/**
	 * Returns the angle between the forward angles of two unit vectors. 
	 * Use angleBetween() for vectors that are not normalized. This function is only included 
	 * to occasionally save a couple of internal normalize calls.<br>
	 * <br>
	 * The result is always positive. If you also need to know the rotational direction 
	 * (clockwise or counter-clockwise) you need to calculate the angle of each vector 
	 * and work it out from there.
	 * 
	 * @shortdesc Returns the angle between two unit vectors.
	 * @param vector1,vector2 (<code>Vect2,Vect2</code>)
	 * @return angle in radians, or NaN if vectors are parallel (<code>float</code>)
	 * @related angleBetween ( )
	 * @example Vect2_angle_between_units
	 */
	public static float angleBetweenUnits( Vect2 vector1, Vect2 vector2 ) {
		return (float) Math.acos( dot( vector1, vector2 ) );
	}
	
	
	
	/////////////
	// HELPERS //
	/////////////
	
	
	/**
	 * Sets the vector by copying (no reference is kept).
	 * 
	 * @shortdesc Sets the vector by copying.
	 * @param	x,y components (<code>float,float</code>)
	 * @param	vector to be copied into the vector (<code>Vect2</code>)
	 * @related setZero ( )
	 * @example Vect2_set
	 */
	public void set( float x, float y ){
		this.x = x;
		this.y = y;
	}
	public void set( Vect2 v ){
		x = v.x;
		y = v.y;
	}

	
	/**
	 * Sets the vector to zero (<code>0,0</code>).
	 * 
	 * @related set ( )
	 * @example Vect2_set_zero
	 */
	public void setZero(){
		x = 0f;
		y = 0f;
	}
	
	
	/**
	 * Returns a copy of the vector. Use this when you want to avoid creating
	 * references between vectors; instead of doing <code>v1 = v2</code>, do 
	 * <code>v1 = v2.copy</code>
	 * 
	 * @shortdesc Returns a copy of the vector.
	 * @return new copy of this vector (<code>Vect2</code>)
	 * @example Vect2_copy
	 */
	public Vect2 copy(){
		return new Vect2( x, y );
	}
	
	
	/**
	 * Returns a string containing the (<code>x,y</code>) components of this vector. 
	 * 
	 * @returns string describing this vector
	 * @example Vect2_to_string
	 */
	public String toString() {
		return "Vect2( " + x + ", " + y + " )";
	}
	
}
