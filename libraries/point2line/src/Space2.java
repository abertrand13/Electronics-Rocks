package point2line;


/**
 * Space2 is a class that compiles a bunch of useful functions 
 * for calculating geometry in 2D space and is only meant to be 
 * accessed statically.<br>
 * <br>
 * Maintained by <a href="http://sixthsensor.dk/">Carl Emil Carlsen</a>
 * 2005+ <br>
 * <br>
 * Space2 is part of the point2line library. The library is free software; you can redistribute it 
 * and/or modify it for any desirable purpose. It is distributed in the hope that 
 * it will be useful, but without any warranty.<br>
 * 
 * @author <a href="http://sixthsensor.dk/">Carl Emil Carlsen</a>
 * @usage Web & Application
 */

// @TODO
// Vect2[] findOutline( Vect2[] points )
// int[] triangulate( Vect2[] polygon )


public class Space2
{
	private static final float PI = (float) Math.PI;
	private static final float TWO_PI = (float) Math.PI * 2f;
	private static final float HALF_PI = (float) Math.PI * 0.5f;
	
	
	private Space2(){}
	
	
	/**
	 * Calculates the point on an ellipse at a specific angle.
	 * Only use this function when absolutely necessary; it is slow. For
	 * the purpose of drawing an ellipse, simply calculate the points on
	 * a circle and scale the x and y value.
	 * <a href="http://mathforum.org/library/drmath/view/54922.html">http://mathforum.org/library/drmath/view/54922.html</a>
	 * 
	 * @shortdesc Calculates the point on an ellipse at a specific angle.
	 * @param angle in radians (<code>float</code>)
	 * @param width of the ellipse (<code>float</code>)
	 * @param height of the ellipse (<code>float</code>)
	 * @return position (<code>Vect2</code>)
	 * @example Space2_ellipse_point
	 */
	public static Vect2 ellipsePoint( float angle, float width, float height )
	{
		angle %= TWO_PI;
		float t = (float) Math.atan( width * Math.tan( angle ) / height);
		if( angle > HALF_PI && angle < PI + HALF_PI ) t -= PI;
		Vect2 result = new Vect2( width * 0.5f * (float) Math.cos( t ), height * 0.5f * (float) Math.sin( t )  );
		return result;
	}
	
	
	/**
	 * Calculates the distance from a point to a line.<br>
	 * <br>
	 * The distance is positive on the right side of the line (following point A to B),
	 * and negative on the other side.<br>
	 * <br>
	 * The distance is calculated as if the line is continues infinitely in both ends.
	 * 
	 * @shortdesc Calculates the distance from a point to a line.
	 * @param point (<code>Vect2</code>)
	 * @param linePointA (<code>Vect2</code>)
	 * @param linePointB (<code>Vect2</code>)
	 * @return distance (<code>float</code>)
	 * @example Space2_point_to_line_distance
	 */
	public static float pointToLineDistance( Vect2 point, Vect2 linePointA, Vect2 linePointB )
	{
		float tmpFloat = ( ( linePointA.y-linePointB.y ) * point.x ) + ((linePointB.x-linePointA.x ) * point.y ) + ( (linePointA.x*linePointB.y ) - ( linePointB.x*linePointA.y ) );
		tmpFloat = (float) ( tmpFloat / ( Math.sqrt ( ( ( linePointB.x-linePointA.x ) * ( linePointB.x-linePointA.x ) ) + ( ( linePointB.y-linePointA.y ) * ( linePointB.y-linePointA.y ) ) ) ) );
		return tmpFloat;
	}
	
	
	/**   
	 * Calculates whether a point is inside a polygon.<br>
	 * <br>
	 * Code is borrowed from this <a href="http://processing.org/discourse/yabb_beta/YaBB.cgi?board=Programs;action=display;num=1189178826">Processing.org 
	 * thread</a>. Thanks to <a href="http://www.robotacid.com/">st33d</a> for the code. 
	 * 
	 * @shortdesc Calculates whether a point is inside a polygon.
	 * @param point containing the x,y coordinates to test (<code>Vect2</code>)
	 * @param vertices defining the polygon in the correct order (<code>Vect2[]</code>)
	 * @return true if the point is inside the polygon, otherwise false (<code>boolean</code>)
	 * @example Space2_inside_polygon
	 */
	public static boolean insidePolygon( Vect2 point, Vect2[] vertices ){
		float[] poly = new float[ vertices.length * 2 ];
		int p = 0;
		for(int i = 0; i < vertices.length; i++) {
			poly[ p++ ] = vertices[ i ].x;
			poly[ p++ ] = vertices[ i ].y;
		}
		return insidePolygon( point.x, point.y, poly );
	}
	private static boolean insidePolygon( float x, float y, float[] poly )
	{
		int pnum=poly.length/2;
		int id1,id2;
		int i, j, c = 0;
		for (i = 0, j = pnum-1; i < pnum; j = i++) {
			id1=i*2;
			id2=j*2;
			if(		(((poly[id1+1] <= y) && (y < poly[id2+1])) ||
					((poly[id2+1] <= y) && (y < poly[id1+1]))) &&
					(x < (poly[id2] - poly[id1]) * (y - poly[id1+1]) / (poly[id2+1] - poly[id1+1]) + poly[id1])
			){
				c = (c+1)%2;
			}
		}
		return c == 1;
	}
	
	
	/**   
	 * Calculates the intersection of two lines. <br>
	 * <br>
	 * The code comes from a post on Code & Form by Marius Watz, which led to a 
	 * response with a more efficient algorithm taken from Graphics Gems. See
	 * the <a href="http://workshop.evolutionzone.com/2007/09/10/code-2d-line-intersection/">original 
	 * discussion</a>.
	 * 
	 * @shortdesc Calculates the intersection of two lines.
	 * @param p1,p2,p3,p4 two pair of vectors each defining a line (<code>Vect2,Vect2,Vect2,Vect2</code>)
	 * @return null if there is no intersection, otherwise a vector containing the intersection coordinate (<code>Vect2</code>)
	 * @example Space2_line_intersection
	 */
	public static Vect2 lineIntersection( Vect2 p1, Vect2 p2, Vect2 p3, Vect2 p4 )
	{
		return lineIntersection( p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y );
	}
	private static Vect2 lineIntersection( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4 )
	{
		float a1, a2, b1, b2, c1, c2;
		float r1, r2, r3, r4;
		float denom, offset, num;	
		
		// Compute a1, b1, c1, where line joining points 1 and 2
		// is "a1 x + b1 y + c1 = 0".
		a1=y2-y1;
		b1=x1-x2;
		c1=(x2*y1)-(x1*y2);
		
		// Compute r3 and r4.
		r3=((a1*x3)+(b1*y3)+c1);
		r4=((a1*x4)+(b1*y4)+c1);

		// Check signs of r3 and r4. If both point 3 and point 4 lie on
		// same side of line 1, the line segments do not intersect.
		if ((r3!=0)&&(r4!=0)&&same_sign(r3, r4)) return null;

		// Compute a2, b2, c2
		a2=y4-y3;
		b2=x3-x4;
		c2=(x4*y3)-(x3*y4);

		// Compute r1 and r2
		r1=(a2*x1)+(b2*y1)+c2;
		r2=(a2*x2)+(b2*y2)+c2;

		// Check signs of r1 and r2. If both point 1 and point 2 lie
		// on same side of second line segment, the line segments do
		// not intersect.
		if ((r1!=0)&&(r2!=0)&&(same_sign(r1, r2))) return null;


		//Line segments intersect: compute intersection point.
		denom=(a1*b2)-(a2*b1);

		if (denom==0) return null;

		if (denom<0) {
			offset=-denom/2;
		} else {
			offset=denom/2;
		}

		// The denom/2 is to get rounding instead of truncating. It
		// is added or subtracted to the numerator, depending upon the
		// sign of the numerator.
		
		Vect2 intersection = new Vect2();
		
		num=(b1*c2)-(b2*c1);
		if (num<0) {
			intersection.x =(num-offset)/denom;
		} else {
			intersection.x =(num+offset)/denom;
		}

		num=(a2*c1)-(a1*c2);
		if (num<0) {
			intersection.y =(num-offset)/denom;
		} else {
			intersection.y =(num+offset)/denom;
		}

		return intersection;
	}
	private static boolean same_sign( float a, float b ){
		return ( ( a * b ) >= 0 );
	}
	
	
	

	
}
