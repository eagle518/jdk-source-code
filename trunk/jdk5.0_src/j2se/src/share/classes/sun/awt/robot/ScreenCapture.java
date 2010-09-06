/*
 * @(#)ScreenCapture.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.robot;

import java.awt.*;
import java.awt.image.*;
import java.io.*;

/**
 * Class representing an image captured directly from
 * the screen buffer. Provides comparison and persistence
 * methods.
 *
 * @version 	@(#)ScreenCapture.java	1.7 03/12/19
 * @author 	Robi Khan
 */
public class ScreenCapture {
    private Image	image;
    
    /**
     * Package level constructor called from Robot class
     */
    ScreenCapture(Image image) {
	this.image = image;
    }
    
    /**
     * Compares screen capture to another capture and
     * determines they are equal on a pixel-by-pixel
     * basis. (x,y) origin is not taken into account.
     */
    public boolean equals(Object other) {
	ScreenCapture	otherCapture;
	if ( !(other instanceof ScreenCapture) ) {
	    return false;
	}

	otherCapture = (ScreenCapture)other;
	Image otherImage = otherCapture.getImage();

	//
	// get pixel buffers for both images and compare for equality
	//
	if ( otherImage.getWidth(null) != image.getWidth(null) ||
	     otherImage.getHeight(null) != image.getHeight(null) ) {
	    // dimensions don't match, so images are obviously different
	    return false;
	}

	int	pixels[] = getPixels(image);
	int	otherPixels[] = getPixels(otherImage);

	// pixel by pixel value comparison
	for ( int nPixel = 0; nPixel < pixels.length; nPixel++ ) {
	    if ( pixels[nPixel] != otherPixels[nPixel] ) {
		return false;
	    }
	}

	return true;
    }

    private int [] getPixels(Image image) {
	int		width = image.getWidth(null);
	int		height = image.getHeight(null);
	int		pixels[] = new int[width*height];
	PixelGrabber	grabber = new PixelGrabber(image, 0, 0, width, height, pixels, 0, width);

	try {
	    grabber.grabPixels();
	} catch (InterruptedException ie) {
	    // should never be a problem since we get all our pixels at once
	    throw new RuntimeException("Problem grabbing pixels.");
	}
	
	return pixels;
    }

    /**
     * Gets the underlying Image in the screen capture
     */
    public Image getImage() {
	return image;
    }

    /**
     * Writes image in GIF format to an output stream?
     * TBD: Do we need a 24-bit lossless format instead?
     */
    public void writeAsGifToStream(OutputStream outputStream) throws IOException {
	throw new Error("NOT IMPLEMENTED");
    }
}
