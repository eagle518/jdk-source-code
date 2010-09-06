/*
 * @(#)ScreenCaptureProducer.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

 package sun.awt.robot;

 import java.awt.*;
 import java.util.*;
 import java.awt.image.*;

/**
 * Implementation of ImageProducer that turns an array
 * of RGB pixel values into an image.
 *
 * @version 	@(#)ScreenCaptureProducer.java	1.7 03/12/19
 * @author 	Robi Khan
 */
class ScreenCaptureProducer implements ImageProducer
{
    Rectangle	captureRect;
    Vector	consumers = new Vector();
    int		pixels[];

    ScreenCaptureProducer(int pixels[], Rectangle screenRect) {
	captureRect = new Rectangle(screenRect.width, screenRect.height);
	this.pixels = pixels;
    }

    public void addConsumer(ImageConsumer ic) {
	consumers.addElement(ic);
    }

    public void removeConsumer(ImageConsumer ic) {
	consumers.removeElement(ic);
    }

    public boolean isConsumer(ImageConsumer ic) {
	return consumers.contains(ic);
    }

    public void requestTopDownLeftRightResend(ImageConsumer ic) {
	// not implemented yet
    }

    public void startProduction(ImageConsumer ic) {
	if ( !isConsumer(ic) ) {
	    addConsumer(ic);
	}

	ColorModel cm =  ColorModel.getRGBdefault();
	ic.setDimensions(captureRect.width, captureRect.height);
	ic.setColorModel(cm);
	ic.setHints(ImageConsumer.SINGLEPASS|ImageConsumer.SINGLEFRAME);
	Properties improps = new Properties();
	improps.setProperty("producer", ScreenCaptureProducer.class.getName());
	ic.setProperties(improps);

	ic.setPixels(0, 0, captureRect.width, captureRect.height, cm, pixels, 0, captureRect.width);
	ic.imageComplete(ImageConsumer.STATICIMAGEDONE);
	consumers.removeElement(ic);
    }
}
