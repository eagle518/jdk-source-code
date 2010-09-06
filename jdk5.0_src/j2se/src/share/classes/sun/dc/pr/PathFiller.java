/*
 * @(#)PathFiller.java	1.20 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)PathFiller.java 3.2 97/11/19
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

package sun.dc.pr;

import	sun.dc.path.*;

/**
 *  A PathFiller is an object capable of turning a <b>path</b>
 *  description into rectangular arrays of alpha pixels as indicated
 *  by the <b>fill mode</b>.
 *  <p>
 *  A <b>fill cycle</b> - the sequence of method invocations
 *  used by a client to decribe path and fill mode, and to retrieve the
 *  resulting alphas - must comform to the following protocol:
 *  <p>
 *  <b>Initialization:</b> The client declares the fill mode - e.g.
 *  non-zero filled or parity filled. The PathFiller stores this
 *  information.
 *  <p>
 *  <b>Path Description:</b> The client describes the path - in raster
 *  space - to the PathFiller, which stores it as part of its state.
 *  <p>
 *  <b>Path Bounding Box Retrieval:</b> At this stage, the client may
 *  request the filler to return the path's bounding box.
 *  <p>
 *  <b>Definition of Output Area:</b> The client defines an
 *  arbitrary rectangle in raster space, the "output area" or OA.
 *  Only the alpha pixels inside this area will be computed.
 *  <p>
 *  <b>Tile Retrieval:</b> OA is divided in square "tiles." The tiles
 *  are retrieved one by one, in a conventional order. After the last
 *  tile is retrieved, the path filler object resets itself to an
 *  initial state and can be used for a different path/fillmode
 *  combination.
 *  <p>
 *  A fill cycle can be interrupted at any time by invoking <b>reset</b>.
 *
 * @version 3.0, July 1997
 * @version 3.2, November 1997
 * @since   2.0
 */
public class PathFiller implements PathConsumer
{
//_____________________________________________________________________
//
//  PUBLIC CONSTANTS
//	fill modes, tile states, sizes
//  CONSTRUCTOR
//	public constructor
//  INITIALIZATION
//	private state, fillmode and related methods and constants
//	public method setFillMode
//  DEFINITION OF OUTPUT AREA
//	private pathBox
//	public method getAlphaBox
//	private static class Run, related constants
//	private tileRuns array, etc
//	private output area positions, dimensions, etc
//	/*private*/ methods processToRunsArc1/2/3
//	private method  runCheckForArcAppend,
//	private methods appendToRunARc1/2/3
//	private runsBuilder (RunsBuilder implements PathConsumer)
//	public method setOutputArea
//  TILE RETRIEVAL
//	private static class LeftSide
//	private lsAffects
//	public method nextTile
//	public method getTileState
//	private fastOutputPC (FastOutputPC implements PathConsumer)
//	private method sendTileToLLFiller
//	public methods writeAlpha
//	public method reset
//_____________________________________________________________________


//  ___________________________________________________________________
//
//  PUBLIC CONSTANTS
//  ___________________________________________________________________

    public static final int
	EOFILL	= 1,			// 
	NZFILL	= 2,			// explicit in jc case

	MAX_PATH = 1000000,		// maximum path coordinate allowed

	TILE_IS_ALL_0	= 0,		// tile states
	TILE_IS_ALL_1	= 1,
	TILE_IS_GENERAL = 2;

    /* This field should be package private to avoid external modification */
    /* public */ static /* final */ int
			tileSizeL2S;	// to be set by c filler at static init time

    private static /* final */ int
			tileSize;	// to be set by c filler at static init time

    private static /* final */ float
			tileSizeF;	// to be set by c filler at static init time

    public static final float
			maxPathF = (float)MAX_PATH;
    public static final boolean
			validLoCoord(float c) { return c >= -maxPathF; }
    public static final boolean
			validHiCoord(float c) { return c <=  maxPathF; }

//  ___________________________________________________________________
//
//  CONSTRUCTOR
//  ___________________________________________________________________

    public PathFiller() {
	cInitialize();
	reset();
    }
    public native void dispose();
    protected static void classFinalize() throws Throwable {
	cClassFinalize();
    }

    public PathConsumer getConsumer() {
	return null;
    }

//  ___________________________________________________________________
//
//  INITIALIZATION
//  ___________________________________________________________________

    /**
     * Determines whether the path will be EOFILLed or NZFILLed.
     * Mandatory; must be the first method used in a fill cycle.
     * <p>
     * @param <tt>fillmode</tt>
     *  the fillmode selector
     * @exception PRError
     *  when invoked <ol>
     *  <li>    in a state other than between fill cycles (unexpected),
     *  <li>    with an invalid selector (unknown fill mode).
     *  </ol>
     */
    public native void setFillMode(int fillmode) throws PRError;

//  ___________________________________________________________________
//
//  PATH DESCRIPTION
//  ___________________________________________________________________
//
    /**
     *  Ends the mode description and begins the path description.
     *  Mandatory.
     *
     * @exception PathError
     *  when invoked <ol>
     *  <li>    before <tt>setFillMode</tt> or more than once in a fill cycle
     *	    (unexpected),
     *  </ol>
     */
    public native void beginPath() throws PathError;

    /**
     *  Begins a new subpath. Mandatory after <tt>beginPath</tt>; and everytime
     *  when a new subpath begins.
     *
     * @param <tt>x0</tt>
     *  the X coordinate of the initial point
     * @param <tt>y0</tt>
     *  the Y coordinate of the initial point
     * @exception PathError
     *  when invoked before <tt>beginSubpath</tt> or after <tt>endPath</tt>
     *  (unexpected).
     */
    public native void beginSubpath(float x0, float y0) throws PathError;

    /**
     *  Appends a line arc to the current subpath. Optional.
     *
     * @param <tt>x1</tt>
     *  the X coordinate of the end point
     * @param <tt>y1</tt>
     *  the Y coordinate of the end point
     * @exception PathError
     *  when invoked before <tt>beginSubpath</tt> or after <tt>endPath</tt>
     *  (unexpected).
     */
    public native void appendLine(float x1, float y1) throws PathError;

    /**
     *  Appends a quadratic arc to the current subpath. Optional.
     *
     * @param <tt>xm</tt>
     *  the X coordinate of the control point;
     * @param <tt>ym</tt>
     *  the Y coordinate of the control point;
     * @param <tt>x1</tt>
     *  the X coordinate of the end point;
     * @param <tt>y1</tt>
     *  the Y coordinate of the end point;
     * @exception PathError
     *  when invoked before <tt>beginSubpath</tt> or after <tt>endPath</tt>
     *  (unexpected)
     */
    public native void appendQuadratic(float xm, float ym, float x1, float y1)
	throws PathError;

    /**
     *  Appends a cubic arc to the current subpath. Optional.
     *
     * @param <tt>xm</tt>
     *  the X coordinate of the 1st control point;
     * @param <tt>ym</tt>
     *  the Y coordinate of the 1st control point;
     * @param <tt>xn</tt>
     *  the X coordinate of the 2nd control point;
     * @param <tt>yn</tt>
     *  the Y coordinate of the 2nd control point;
     * @param <tt>x1</tt>
     *  the X coordinate of the end point;
     * @param <tt>y1</tt>
     *  the Y coordinate of the end point;
     * @exception PathError
     *  when invoked before <tt>beginSubpath</tt> or after <tt>endPath</tt>
     *  (unexpected)
     */
    public native void appendCubic(float xm, float ym, float xn, float yn, float x1, float y1)
	throws PathError;

    /**
     *  A non operational method (all subpaths will be closed regardless).
     *  Optional.
     *
     * @exception PathError
     *  when invoked before <tt>beginSubpath</tt> or after <tt>endPath</tt>
     *  (unexpected)
     */
    public native void closedSubpath() throws PathError;

    /**
     *  Ends the path description. Mandatory.
     *
     * @exception PathError
     *  when invoked before <tt>beginPath</tt> or more than once in a fill cycle
     *  (unexpected).
     * @exception PathException
     *  when the path would result in non-zero alpha values for pixel
     *  coordinates outside of [<tt>-MAX_ALPHA,MAX_ALPHA</tt>] (alpha coordinate
     *  out of bounds).
     */
    public native void endPath() throws PathError, PathException;

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

//  ___________________________________________________________________
//
//  DEFINITION OF THE OUTPUT AREA
//  ___________________________________________________________________


    /**
     * Returns in its argument array a box guaranteed to contain all arcs
     * in the path. Optional. May be invoked anytime after path
     * description.
     *
     * @param <tt>box</tt>
     *  an array of no less than 4 entries where the extreme coordinates
     *  of the alpha box are placed;
     *
     * @exception PRError
     *  when invoked before the path has been defined (unexpected).
     * @exception PathError
     *  when the box is invalid (invalid box destination array).
     */
    public native void getAlphaBox(int[] box) throws PRError;

    /**
     * Declares the output area (OA), a region of raster space
     * containing all the pixels deemed interesting by the client.
     * The pixels will be written by succeeding invocations to
     * <tt>writeAlpha</tt>.
     * <p>
     * Pixels will be retrieved in "tiles", rectangular groups of maximum
     * dimension &lt;= <tt>tileSize</tt>.  The region is tiled with maximal
     * square tiles organized in "rows" extending in the direction X+. If
     * we visualize the coordinate axii X/Y respectively pointing
     * right/up, then the first tile is placed with its lower/left corner
     * coincident with the lower/left corner of the output area. The
     * first row consists of the first tile and possibly others placed to
     * its right, as needed to completely cover the width of the output
     * area. Rows are stacked above the first row as needed to
     * completely cover the height of the output area. Tiles do not
     * exceed OA; because the width and height of OA are not, in
     * general, multiples of <tt>tileSize</tt>, the rightmost tiles may
     * have a width &lt; <tt>tileSize</tt> and the uppermost tiles a height
     * &lt; <tt>tileSize</tt>.
     * <p>
     * The method is mandatory. It must be used at least once, after the
     * description of the path and before the retrival of tiles can
     * begin.
     *
     * @param <tt>x0</tt>
     *  the low X boundary of the output area;
     * @param <tt>y0</tt>
     *  the low Y boundary of the output area;
     * @param <tt>w</tt>
     *  the (>0) X dimension of the output area;
     * @param <tt>h</tt>
     *  the (>0) Y dimension of the output area;
     * @exception PRError
     *  when invoked <ol>
     *  <li>    before defining the path (unexpected),
     *  <li>    with a statically invalid output area (<tt>w&lt;=0 | h&lt;=0</tt>)
     *          (invalid output area),
     *  </ol>
     * @exception PRException
     *  when <tt>x0</tt>, <tt>y0</tt>, <tt>x0+w</tt> or <tt>y0+h</tt> fall outside
     *  [<tt>-MAX_ALPHA,MAX_ALPHA</tt>] (alpha coordinate out of bounds).
     */
    public native void setOutputArea(float outlox, float outloy, int w, int h)
	throws PRError, PRException;
 
//  ___________________________________________________________________
//
//  TILE RETRIEVAL
//  ___________________________________________________________________

    /**
     *  Returns the state of the current tile. Optional.
     * @return either TILE_IS_ALL_0, TILE_IS_ALL_1 or TILE_IS_GENERAL
     *
     * @exception PRError
     *  when invoked before <tt>setOutputArea</tt> (unexpected).
     */
    public native int getTileState() throws PRError;

    /**
     * Writes the alpha pixels of the current tile to pixel destination
     * declared - the rectangular array alpha; this array is organized
     * so the index difference between two pixels is <tt>xstride</tt>
     * for X-adjacent pixels and <tt>ystride</tt> for Y-adjacent pixels.
     * The pixel of lowest coordinates in the tile is written to the alpha
     * array position <tt>pix0offset</tt>, depending on the alpha values -
     * reals between 0 and 1 - are scaled by 255 (<tt>byte[] alpha</tt>)
     * or 65535 (<tt>char[] alpha</tt>).
     *
     * @param <tt>alpha</tt>
     *  the array where pixels is to be placed;
     * @param <tt>xstride</tt>
     *  the index difference in the array of alphas (>0) between two
     *  pixels adjacent in the X direction;
     * @param <tt>ystride</tt>
     *  the index difference in the array of alphas (>0) between two
     *  pixels adjacent in the Y direction;
     * @param <tt>pix0ffset</tt>
     *  the offset (>=0) of the lowest coordinates pixel in the array
     *  alpha.
     * @exception PRError
     *  when invoked <ol>
     *  <li>    before <tt>setOutputArea</tt> (unexpected),
     *  <li>    with <tt>alpha==null</tt>, 
     * 	    <tt>xstride&lt;=0, ystride&lt;=0</tt> or
     * 	    <tt>pix0offset&lt0 (invalid alpha destination).
     *  </ol>
     * @exception PRException
     *  when the combination of the pixel destination parameters
     *  (<tt>alpha.length</tt>, <tt>xstride</tt> and <tt>ystride</tt>),
     *  <tt>pix0offset</tt> and dimensions of the current tile would
     *  otherwise result in an <tt>IndexOutOfBoundsException</tt>
     *  (alpha destination array too short).
     */
    public void writeAlpha(byte[] alpha, int xstride, int ystride, int pix0offset)
		throws PRError, PRException, InterruptedException {
	writeAlpha8(alpha, xstride, ystride, pix0offset);
    }
    /**
     *  See writeAlpha(byte[] ...)
     */
    public void writeAlpha(char[] alpha, int xstride, int ystride, int pix0offset)
		throws PRError, PRException, InterruptedException {
	writeAlpha16(alpha, xstride, ystride, pix0offset);
    }

    private native void writeAlpha8(byte[] alpha, int xstride, int ystride, int pix0offset) throws PRError, PRException;
    private native void writeAlpha16(char[] alpha, int xstride, int ystride, int pix0offset) throws PRError, PRException;

    /**
     *  Advances to the next tile. Optional (but either <tt>writeAlpha</tt> or
     *  <tt>nextTile</tt> must be used in order to move to the next tile).
     * @exception PRError
     *  when invoked before <tt>setOutputArea</tt> (unexpected).
     */
    public native void nextTile() throws PRError;
    
    /**
     *  Mandatory; must be invoked to end a rasterization cycle.
     *  Can be invoked anytime.
     */
    public native void reset();


//  ___________________________________________________________________
//
//  CONNECTIONS to PathFiller in C
//  ___________________________________________________________________
    static {
	java.security.AccessController.doPrivileged(
		  new sun.security.action.LoadLibraryAction("dcpr"));
	cClassInitialize();
    }
    private static native void	cClassInitialize();
    private static native void	cClassFinalize();

    private		  long	cData;
    private	   native void	cInitialize();
}
