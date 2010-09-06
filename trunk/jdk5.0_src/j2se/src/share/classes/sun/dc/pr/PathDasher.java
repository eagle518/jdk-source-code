/*
 * @(#)PathDasher.java	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)PathDasher.java 3.2 97/11/19
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

package sun.dc.pr;

import sun.dc.path.*;

/**
 * A PathDasher takes certain "dash parameters" and an input path, and
 * computes an output path whose subpaths correspond to the dashes
 * defined by the parameters on the input. This path is sent - previously
 * transformed by [outt6] - to the PathCOnsumer [dest] associated with
 * the PathDasher at creation time.
 *
 * @version 3.0, July 1997
 * @version 3.2, November 1997
 * @since   2.0
 */
public class PathDasher implements PathConsumer {

//  ____________________________________________________________________________
//
//  PUBLIC CONSTANTS
//  ____________________________________________________________________________

//  ____________________________________________________________________________
//
//  CONSTRUCTOR and DESTRUCTOR
//  ____________________________________________________________________________

    private PathConsumer dest;

    public PathDasher(PathConsumer dest) {
	this.dest = dest;
	cInitialize(dest);
	reset();
    }
    public native void	dispose();
    protected static void	classFinalize() throws Throwable {
	cClassFinalize();
    }

    public PathConsumer getConsumer() {
	return dest;
    }

//  ____________________________________________________________________________
//
//  PATH DASHER METHODS
//  ____________________________________________________________________________

    private static final float	TOP_MAX_MIN_RATIO = 100F;

    /**
     *  Sets the dash pattern and its initial offset. Optional before
     *  <code>beginPath</code>. The default pattern consists of a single,
     *  infinitely long dash, resulting in an output path identical to
     *  the input path (transformed by the output transformation).
     *
     * @param <code>dash</code>
     *  the dash pattern; its values are lengths, alternatively
     *  interpreted as dash and interdash lengths (the actual length
     *  depends also on their direction and on the transformation set by
     *  <code>setDashT4</code>); null stands for a solid line
     * @param <code>offset</code>
     *  the initial offset in the dash pattern
     * @exception PRError
     *  when invoked <ol>
     *  <li>    after <code>beginPath</code> (unexpected),
     *  <li>    with a negative offset, negative dash, negative interdash lengths,
     *          or with a dash pattern consisting only of zeroes
     *          (invalid dash pattern).
     *  </ol>
     */
    public native  void	setDash(float[] dash, float offset) throws PRError;

    /**
     *  Sets the transformation which transforms the dash pattern defined
     *  by <code>setDash</code>.
     *  Optional before <code>beginPath</code> (default = identity).
     *
     * @param <code>t4</code>
     *  a 4 coefficient transformation; null stands for the identity transformation;
     * @exception PRError
     *  when invoked <ol>
     *  <li>    after <code>beginPath</code> (unexpected),
     *  <li>    with <code>t4.length &lt; 4</code> (invalid dash transformation),
     *  <li>    with a non-inversible t4 (invalid dash transformation (singular)).
     *  </ol>
     */
    public native  void	setDashT4(float[] t4) throws PRError;

    /**
     *  Sets the output transformation which transforms the dashed input path
     *  before it is sent to the destination consumer.
     *  Optional before <code>beginPath</code> (default = identity).
     *
     * @param <code>t6</code>
     *  a 6 coefficient transformation; null stands for the identity transformation;
     * @exception PRError
     *  when invoked <ol>
     *  <li>    after <code>beginPath</code> (unexpected),
     *  <li>    with <code>t6.length &lt; 6</code> (invalid output transformation),
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
     *  Leaves the dasher ready to begin a new dash cycle.
     *  Optional. Can be invoked anytime.
     */
    public native  void	reset();

//  ____________________________________________________________________________
//
//  PATHCONSUMER IMPLEMENTATION
//  ____________________________________________________________________________

    /**
     *  Ends the dash description and begins the path description. It is at
     *  this point that the dash is validated. Mandatory.
     *
     * @exception PathError
     *  when invoked <ol>
     *  <li>    more than once in a dash cycle (unexpected),
     *  <li>    with a statically invalid box (invalid path box).
     *  </ol>
     */
    public native  void	beginPath() throws PathError;

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
     *  <li>    before <code>beginSubpath</code> or after <code>endPath</code> (unexpected).
     *  </ol>
     */
    public native  void	beginSubpath(float x0, float y0) throws PathError;

    /**
     *  Appends a line arc to the current subpath. Optional.
     *
     * @param <code>x1</code>
     *  the X coordinate of the end point
     * @param <code>y1</code>
     *  the Y coordinate of the end point
     * @exception PathError
     *  when invoked <ol>
     *  <li>    before <code>beginSubpath</code> or after <code>endPath</code> (unexpected).
     *  </ol>
     */
    public native  void	appendLine(float x1, float y1) throws PathError;

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
     *  <li>	before <code>beginSubpath</code> or
     *		after <code>endPath</code> (unexpected).
     *  </ol>
     */
    public native  void	appendQuadratic(float xm, float ym, float x1, float y1)
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
     *  <li>    before <code>beginSubpath</code> or
     *		after <code>endPath</code> (unexpected).
     *  </ol>
     */
    public native  void	appendCubic(float xm, float ym, float xn, float yn,
				    float x1, float y1) throws PathError;

    /**
     *  Declares that the current subpath will be closed. This may require
     *  to append a line arc connecting the last point to the first.
     *  Optional.
     *
     * @exception PathError
     *  when invoked <ol>
     *  <li>    before <code>beginSubpath</code> or
     *		after <code>endPath</code> (unexpected).
     *  </ol>
     */
    public native  void	closedSubpath()	throws PathError;

    /**
     *  Ends the path description. Mandatory.
     *
     * @exception PathError
     *  when invoked <ol>
     *  <li>    before <code>beginPath</code> or more than once in a dash cycle
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
    
//  ____________________________________________________________________________
//
//  CONNECTIONS to dcPathDasher in C
//  ____________________________________________________________________________
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
