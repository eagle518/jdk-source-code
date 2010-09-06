/*
 * @(#)ImageFetchable.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.image;

/**
 * This interface allows the ImageFetcher class to drive the production
 * of image data in an ImageProducer class by calling the doFetch()
 * method from one of a pool of threads which are created to facilitate
 * asynchronous delivery of image data outside of the standard system
 * threads which manage the applications User Interface.
 *
 * @see ImageFetcher
 * @see ImageProducer
 *
 * @version 	1.11 12/19/03
 * @author 	Jim Graham
 */
public interface ImageFetchable {
    /**
     * This method is called by one of the ImageFetcher threads to start
     * the flow of information from the ImageProducer to the ImageConsumer.
     * @see ImageFetcher
     * @see ImageProducer
     */
    public void doFetch();
}
