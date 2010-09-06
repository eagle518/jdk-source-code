/*
 * @(#)PathStroker.java	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)PathStroker.java 3.2 97/11/19
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

package sun.dc.pr;

import	sun.dc.path.*;

/**
 * A PathStroker takes certain "stroke parameters" and an input path, and
 * computes an output path that corresponds to the "nonzero-winding
 * outline" of the stroked input path; this path it sends to the
 * PathConsumer [dest] permanently associated with the PathStroker since
 * its creation. It is essential that "nonzero-winding" rules be used to
 * determine containment, fill or use the output path in any form: the
 * path will generally be self intersecting and define regions whose
 * winding number [wn] is greater than 1 in absolute value.
 *
 * Notice that, in addition to the pen transformation [pent4], we now
 * also include an [outt6]. This transformation is applied to the stroked
 * outline before sending it to [dest]. Strictly speaking it is always
 * possible to obtain a given result without resorting to [outt6], but
 * its presence makes certain cases easier to express.
 *
 * @version 3.0, July 1997
 * @version 3.2, November 1997
 * @since   2.0
 */
public class PathStroker implements PathConsumer {

//  ____________________________________________________________________
//
//  PUBLIC CONSTANTS
//  ____________________________________________________________________

    public static final int	ROUND		= 10,	// caps, corners
				SQUARE		= 20,
				BUTT		= 30,
				BEVEL		= 40,
				MITER		= 50;

//  ____________________________________________________________________
//
//  CONSTRUCTOR and DESTRUCTOR
//  ____________________________________________________________________

    private PathConsumer dest;

    public	PathStroker(PathConsumer dest) {
	this.dest = dest;
	cInitialize(dest);
	reset();
    }
    public native void dispose();
    protected static void classFinalize() throws Throwable {
	cClassFinalize();
    }

    public PathConsumer getConsumer() {
	return dest;
    }

//  ____________________________________________________________________
//
//  PATH STROKER METHODS
//  ____________________________________________________________________

    /**
     *  Sets the diameter of the pen used in stroking (the actual size and
     *  shape of the raster pen depends also on the transformation set by
     *  <code>setPen(T4)</code>.
     *  Optional before <code>beginPath</code> (default = 1).
     *
     * @param <code>d</code>
     *  the diameter of the pen
     * @exception PRError
     *  when invoked <ol>
     *  <li>    after <code>beginPath</code> (unexpected),
     *  <li>    with <code>d &lt; 0</code> (invalid pen diameter).
     *  </ol>
     */
    public native void	setPenDiameter(float d)	throws PRError;

    /**
     *  Sets the transformation which transforms the circle defined by
     *  <code>setPenDiameter</code> into the (elliptical) raster pen.
     *  Optional before <code>beginPath</code> (default = identity).
     *
     * @param <code>t4</code>
     *  a 4 coefficient transformation; null stands for the identity transformation
     * @exception PRError
     *  when invoked <ol>
     *  <li>    after <code>beginPath</code> (unexpected),
     *  <li>    with <code>t4.length &lt; 4</code> (invalid pen transformation),
     *  <li>    with a non-inversible t4 (invalid pen transformation (singular)).
     *  </ol>
     */
    public native void	setPenT4(float[] t4) throws PRError;

    /**
     *  Changes the pen transformation to ensure that the ellipse obtained
     *  by applying the output transformation to the raster pen has vertical
     *  and horizontal projection which are (1) multiples of <code>unit</code> and
     *  (2) no smaller than <code>unit*mindiameter</code>.
     *  Optional before <code>beginPath</code>.
     *
     * @param <code>unit</code>
     * @param <code>mindiameter</code>
     * @exception PRError
     *  when invoked <ol>
     *  <li>    after <code>beginPath</code> (unexpected),
     *  <li>    with <code>unit &lt; 0</code> or <code>mindiameter &lt; 0</code>
     *          (invalid pen fitting specification).
     *  </ol>
     */
    public native void	setPenFitting(float unit, int mindiameter) throws PRError;

    /**
     *  Sets the shape of the caps to either BUTT, ROUND or SQUARE.
     *  Optional before <code>beginPath</code> (default: ROUND).
     *
     * @param <code>caps</code>
     *  the caps shape selector
     * @exception PRError
     *  when invoked <ol>
     *  <li>    after <code>beginPath</code> (unexpected),
     *  <li>    with an invalid selector (unknow cap type).
     *  </ol>
     */
    public native void	setCaps(int caps) throws PRError;

    /**
     *  Sets the shape of the corners to either ROUND, BEVEL or MITER.
     *  Optional before <code>beginPath</code> (default: ROUND).
     *
     * @param <code>corners</code>
     *  the corners shape selector
     * @param <code>miter</code>
     *  determines how sharp a mitered corner can be; otherwise irrelevant;
     * @exception PRError
     *  when invoked <ol>
     *  <li>    after <code>beginPath</code> (unexpected),
     *  <li>    with <code>corners==MITER & miter &lt; 0</code> (invalid miter limit).
     *  </ol>
     */
    public native void	setCorners(int corners, float miter) throws PRError;

    /**
     *  Sets the output transformation which transforms the stroked input path
     *  before it is sent to the destination consumer.
     *  Optional before <code>beginPath</code> (default = identity).
     *
     * @param <code>t6</code>
     *  a 6 coefficient transformation; null stands for the identity transformation;
     * @exception PRError
     *  when invoked <ol>
     *  <li>    after <code>beginPath</code> (unexpected),
     *  <li>    with <code>t6.length &lt; 6</code> (invalid output transformation);
     *  <li>    with a non-inversible t6 (invalid output transformation (singular)).
     *  </ol>
     */
    public native void	setOutputT6(float[] t6) throws PRError;

    /**
     *  Resets the destination path consumer.
     *  Optional before <code>beginPath</code>.
     *
     * @param <code>dest</code>
     *  the destination path consumer.
     * @exception PRError
     *  when invoked <ol>
     *  <li>    after <code>beginPath</code> (unexpected),
     *  </ol>
     */
    public native void	setOutputConsumer(PathConsumer dest) throws PRError;

    /**
     *  Leaves the stroker ready to begin a new stroke cycle.
     *  Optional. Can be invoked anytime.
     */
    public native void	reset();


//  ____________________________________________________________________
//
//  PATHCONSUMER IMPLEMENTATION
//  ____________________________________________________________________

    /**
     *  Ends the PAC description and begins the path description. It is at
     *  this point that the PAC is validated. Mandatory.
     *
     * @exception PathError
     *  when invoked <ol>
     *  <li>    more than once in a stroke cycle (unexpected),
     *  <li>    with a statically invalid box (invalid path box).
     *  </ol>
     */
    public native void	beginPath() throws PathError;

    /**
     *  Begins a new subpath. Mandatory after <code>beginPath</code>; and everytime
     *  when a new subpath begins.
     *
     * @param <code>x0</code>
     *  the X coordinate of the initial point
     * @param <code>y0</code>
     *  the Y coordinate of the initial point
     * @exception PathError
     *  when invoked <ol>
     *  <li>    before <code>beginSubpath</code> or after <code>endPath</code>
     *          (unexpected).
     *  </ol>
     */
    public native void	beginSubpath(float x0, float y0)throws PathError;

    /**
     *  Appends a line arc to the current subpath. Optional.
     *
     * @param <code>x1</code>
     *  the X coordinate of the end point
     * @param <code>y1</code>
     *  the Y coordinate of the end point
     * @exception PathError
     *  when invoked <ol>
     *  <li>    before <code>beginSubpath</code> or after <code>endPath</code>
     *          (unexpected).
     *  </ol>
     */
    public native void	appendLine(float x1, float y1)	throws PathError;

    /**
     *  Appends a quadratic arc to the current subpath. Optional.
     *
     * @param <code>xm</code>
     *  the X coordinate of the control point;
     * @param <code>ym</code>
     *  the Y coordinate of the control point;
     * @param <code>x1</code>
     *  the X coordinate of the end point;
     * @param <code>y1</code>
     *  the Y coordinate of the end point;
     * @exception PathError
     *  when invoked <ol>
     *  <li>    before <code>beginSubpath</code> or after <code>endPath</code>
     *          (unexpected).
     *  </ol>
     */
    public native void	appendQuadratic(float xm, float ym, float x1, float y1)
				throws PathError;

    /**
     *  Appends a cubic arc to the current subpath. Optional.
     *
     * @param <code>xm</code>
     *  the X coordinate of the 1st control point;
     * @param <code>ym</code>
     *  the Y coordinate of the 1st control point;
     * @param <code>xn</code>
     *  the X coordinate of the 2nd control point;
     * @param <code>yn</code>
     *  the Y coordinate of the 2nd control point;
     * @param <code>x1</code>
     *  the X coordinate of the end point;
     * @param <code>y1</code>
     *  the Y coordinate of the end point;
     * @exception PathError
     *  when invoked <ol>
     *  <li>    before <code>beginSubpath</code> or after <code>endPath</code>
     *          (unexpected).
     *  </ol>
     */
    public native void	appendCubic(float xm, float ym, float xn, float yn,
				    float x1, float y1) throws PathError;

    /**
     *  Declares that the current subpath will be closed. This may require
     *  to append a line arc connecting the last point to the first.
     *  Optional.
     *
     * @exception PathError
     *  when invoked <ol>
     *  <li>    before <code>beginSubpath</code> or after <code>endPath</code>
     *          (unexpected).
     *  </ol>
     */
    public native void	closedSubpath()	throws PathError;

    /**
     *  Ends the path description. Mandatory.
     *
     * @exception PathError
     *  when invoked <ol>
     *  <li>    before <code>beginPath</code> or more than once in a stroke cycle
     *          (unexpected).
     *  </ol>
     * @exception PathException
     *  when the destination consumer throws the exception.
     */
    public native void	endPath() throws PathError, PathException;

    /**
     *	Proxy is ignored in this version; path is sent immediately.
     *	No error checking done, except the one caught by proxy.
     *
     * @exception PathError
     * @exception PathException
     * @see dc.path.FastPathProducer
     */
    public void useProxy(FastPathProducer proxy) throws PathError, PathException {
	proxy.sendTo(this);
    }

    /**
     * Returns the corresponding C path consumer.
     */
    public native long	getCPathConsumer();

//  ____________________________________________________________________
//
//  CONNECTIONS to dcPathStroker in C
//  ____________________________________________________________________
    static {
	java.security.AccessController.doPrivileged(
		  new sun.security.action.LoadLibraryAction("dcpr"));
	cClassInitialize();
    }
    private static native void	cClassInitialize();
    private static native void	cClassFinalize();

    private	   long	cData;
    private native void	cInitialize(PathConsumer dest);
}
