/*
 * @(#)PathConsumer.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)PathConsumer.java 3.2 97/11/19
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

package sun.dc.path;

/**
 * Provides protocols for path description.  Two alternative
 * protocols can be chosen: <i>immediate</i> and <i>by proxy.</i>
 * <p>
 * An <i>immediate path description</i> employs all methods in the
 * interface with the exception of <tt>useProxy.</tt> Such description
 * consists of (an invocation of method) <tt>beginPath</tt>, followed
 * by zero or more <i>immediate subpath descriptions,</i> followed by
 * <tt>endPath</tt>. An immediate subpath description consists of
 * <tt>beginSubpath</tt> followed by zero or more occurrences of
 * <tt>appendLine</tt>, <tt>appendQuadratic</tt>,
 * <tt>appendCubic</tt> and <tt>closedSubpath</tt>.
 * <p>
 * A description <i>by proxy</i> entails invoking <tt>useProxy</tt>
 * with a <tt>FastPathProducer</tt> proxy object representing the
 * path. The <tt>PathConsumer</tt> will store the proxy, eventually
 * direct it to describe the path, and finally forget it.
 * <p>
 * As it does not involve implementing <tt>FastPathProducer</tt>,
 * immediate description is the simplest approach. It is also the one
 * that demands the least from the path source: the path needs to be
 * described just once, the description may take as long as necessary
 * and the path does not need to be known all at once. Description by
 * proxy makes greater demands: a <tt>FastPathProducer</tt> object
 * must be able to produce the path on demand and with minimal
 * overhead, and is expected to know the path's boundaries. Still,
 * description by proxy is recommended over immediate description for
 * path sources that inherently have the necessary abilities.
 * <p>
 * Such preference is based on speed and storage considerations. In
 * some implementations of <tt>PathConsumer</tt>, the consumer may be
 * unable to process the arcs at the time they are sent, or may need
 * to process them repeatedly, or may have to lock a shared, scarce
 * resource to process the path and be unwilling to do so without
 * some assurance that the path generation overhead will be
 * small. Any of these reasons may force the consumer to store a path
 * immediately described. Because descriptions by proxy address all
 * the potential problems and concerns expressed above, they may make
 * such storing unnecessary.
 * <p>
 * A description by proxy will never be processed less efficiently
 * than an equivalent immediate description by any path consumer, but
 * it may not be processed more efficiently either. Therefore, a path
 * source should not strive to implement it when the required
 * abilities are not inherently present. Rather, it should simply
 * describe the path in the less demanding immediate fashion.
 * @see FastPathProducer
 */
public interface PathConsumer {
    /**
     * Begins an immediate path description.
     * @exception PathError
     *  when invoked at an inappropriate time (unexpected).
     */
    public void beginPath() throws PathError;

    /**
     * Begins a new subpath description.
     * @exception PathError
     * when invoked at an inappropriate time (unexpected).
     */
    public void beginSubpath(float x0, float y0) throws PathError;

    /**
     * Extends the current subpath with a line segment.
     * @exception PathError
     *  when invoked at an inappropriate time (unexpected).
     */
    public void appendLine(float x1, float y1) throws PathError;

    /**
     * Extends the current subpath with a quadratic arc segment.
     * @exception PathError
     *  when invoked at an inappropriate time (unexpected).
     */
    public void appendQuadratic(float xm, float ym,
				float x1, float y1) throws PathError;

    /**
     * Extends the current subpath with a cubic arc segment.
     * @exception PathError
     *  when invoked at an inappropriate time (unexpected).
     */
    public void appendCubic(	float xm, float ym,
				float xn, float yn,
				float x1, float y1) throws PathError;

    /**
     * Declares the current subpath to be closed. A subpath is open by default.
     * @exception PathError
     *  when invoked at an inappropriate time (unexpected).
     */
    public void closedSubpath() throws PathError;

    /**
     * Ends an immediate path description
     * @exception PathError
     *  when invoked at an inappropriate time (unexpected).
     * @exception PathException
     *  when the path is invalid from the standpoint of its implementation.
     */
    public void endPath() throws PathError, PathException;

    /**
     * Describes a path <i>by proxy</i>
     * @exception PathError
     *  when invoked at an inappropriate time (unexpected).
     * @see dc.path.FastPathProducer
     */
    public void	useProxy(FastPathProducer proxy) throws PathError, PathException;

    /**
     * Returns the corresponding C object if a native implementation exists;
     * otherwise, returns 0.
     */
    public long	getCPathConsumer();

    /**
     * Disposes any resources, particularly native resources, used
     * by this PathConsumer.  This should only be done when the
     * object is no longer referenced anywhere as the object will
     * be useless once disposed.
     */
    public void dispose();

    /**
     * Returns the next PathConsumer in a chain if this PathConsumer
     * is merely serving as a geometry filter for another.  This is
     * mainly implemented by PathDasher and PathStroker, but is
     * included in the base interface to simplify the action of
     * running down a chain of consumers and acting on them (such
     * as disposing them).
     * <p>
     * An implementation is allowed to return null if it represents
     * a leaf node on a chain of consumers.
     */
    public PathConsumer getConsumer();
}
