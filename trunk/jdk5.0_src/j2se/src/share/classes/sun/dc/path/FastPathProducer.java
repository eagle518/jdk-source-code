/*
 * @(#)FastPathProducer.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)FastPathProducer.java 3.1 97/11/17
 *
 * ---------------------------------------------------------------------
 *	Copyright (c) 1992-1997 by Ductus, Inc. All Rights Reserved.
 * ---------------------------------------------------------------------
 *
 */

package sun.dc.path;

/**
 * This interface should be supported by path sources capable of
 * producing the path's description on demand, multiple times and
 * fast.
 * @see dc.path.PathConsumer
 */
public interface FastPathProducer {

    /**
     * Returns the path's bounding box.
     * @param box
     *  a 4 element array where the coordinates defining the bounding box
     *  are returned
     * @exception PathError
     *  if invoked with an invalid argument - <tt>null</tt> or too short
     */
    public void getBox(float[] box)	throws PathError;

    /**
     * Describes the path for a path consumer.
     * @param pc
     *  the receiving consumer
     * @exception PathError
     *  if invoked with a <tt>null</tt> parameter
     * @exception PathException
     *  if <tt>pc</tt> throws it
     */
    public void sendTo(PathConsumer pc)	throws PathError, PathException;
}
