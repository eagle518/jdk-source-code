/*
 * @(#)SunWritableRaster.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.image;

import java.awt.Point;
import java.awt.Rectangle;
import java.awt.image.DataBuffer;
import java.awt.image.Raster;
import java.awt.image.SampleModel;
import java.awt.image.WritableRaster;


/**
 * This class exists as a middle layer between WritableRaster and its
 * implementation specific subclasses (ByteComponentRaster, ShortBandedRaster,
 * etc).  It provides a means for notifying an associated listener that the
 * contents of the raster have been modified.
 */
public class SunWritableRaster extends WritableRaster {

    protected RasterListener listener;
    private boolean isStolen;

    public SunWritableRaster(SampleModel sampleModel, Point origin) {
        super(sampleModel, origin);
        isStolen = false;
    }

    public SunWritableRaster(SampleModel sampleModel,
                             DataBuffer dataBuffer,
                             Point origin) 
    {
        super(sampleModel, dataBuffer, origin);
        isStolen = true;
    }

    public SunWritableRaster(SampleModel sampleModel,
                             DataBuffer dataBuffer,
                             Rectangle aRegion,
                             Point sampleModelTranslate,
                             WritableRaster parent)
    {
        super(sampleModel, dataBuffer, aRegion, sampleModelTranslate, parent);
        isStolen = true;
    }


    /**
     * Attaches a listener object to this raster
     */
    public void setRasterListener(RasterListener rl) {
        if (rl == null) {
            return;
        }

        boolean wasStolen;
        synchronized (this) {
            if (listener == null) {
                // set the listener; if isStolen was already set to true,
                // we'll notify this new listener about it below
                listener = rl;
                wasStolen = isStolen;
            } else {
                // someone is already listening to this raster, so let's
                // notify the new listener that the raster's already been
                // stolen (the existing listener is not affected)
                wasStolen = true;
            }
        }

        if (wasStolen) {
            // if we've already been stolen, tell this new
            // listener about it immediately
            rl.rasterStolen();
        }
    } 

    /**
     * If a listener exists, sends notification that the raster's contents
     * have been modified
     */
    public void notifyChanged() {
        if (listener != null) {
            listener.rasterChanged();
        }
    }

    /**
     * If a listener exists, sends notification that the raster has been
     * taken in a way that places it out of control of the listner
     */
    public void notifyStolen() {
        setStolen(true);
    }

    /**
     * Sets a flag indicating that the underlying pixels are already out
     * of our control (the developer has a reference to the DataBuffer and/or
     * primitive array).  Useful in cases where a RasterListener has not yet
     * been set, so when it comes time to attach a RasterListener, we
     * will notify that listener that the raster has already been stolen (if
     * the isStolen flag is true).
     *
     * Note that if setStolen(false) is called after the listener has been
     * set, that listener will not be notified of any previous steal attempt.
     * This is not an issue in the current code base, but may be something
     * to keep in mind in the future.
     *
     * @see setRasterListener
     */
    public void setStolen(boolean b) {
        isStolen = b;

        if ((listener != null) && isStolen) {
            listener.rasterStolen();
        }
    }


    /** 
     * Raster/WritableRaster overrides
     */


    public DataBuffer getDataBuffer() {
        notifyStolen();
        return super.getDataBuffer();
    }
    
    public void setDataElements(int x, int y, Object inData) {
        super.setDataElements(x, y, inData);
        notifyChanged();
    }

    public void setDataElements(int x, int y, Raster inRaster) {
        super.setDataElements(x, y, inRaster);
        notifyChanged();
    }

    public void setDataElements(int x, int y, int w, int h, Object inData) {
        super.setDataElements(x, y, w, h, inData);
        notifyChanged();
    }

    public void setRect(Raster srcRaster) {
        super.setRect(srcRaster);
        notifyChanged();
    }

    public void setRect(int dx, int dy, Raster srcRaster) {
        super.setRect(dx, dy, srcRaster);
        notifyChanged();
    }

    public void setPixel(int x, int y, int iArray[]) {
        super.setPixel(x, y, iArray);
        notifyChanged();
    }

    public void setPixel(int x, int y, float fArray[]) {
        super.setPixel(x, y, fArray);
        notifyChanged();
    }

    public void setPixel(int x, int y, double dArray[]) {
        super.setPixel(x, y, dArray);
        notifyChanged();
    }

    public void setPixels(int x, int y, int w, int h, int iArray[]) {
        super.setPixels(x, y, w, h, iArray);
        notifyChanged();
    }

    public void setPixels(int x, int y, int w, int h, float fArray[]) {
        super.setPixels(x, y, w, h, fArray);
        notifyChanged();
    }

    public void setPixels(int x, int y, int w, int h, double dArray[]) {
        super.setPixels(x, y, w, h, dArray);
        notifyChanged();
    }

    public void setSample(int x, int y, int b, int s) {
        super.setSample(x, y, b, s);
        notifyChanged();
    }

    public void setSample(int x, int y, int b, float s){
        super.setSample(x, y, b, s);
        notifyChanged();
    }

    public void setSample(int x, int y, int b, double s){
        super.setSample(x, y, b, s);
        notifyChanged();
    }

    public void setSamples(int x, int y, int w, int h, int b, 
                           int iArray[]) 
    {
        super.setSamples(x, y, w, h, b, iArray);
        notifyChanged();
    }

    public void setSamples(int x, int y, int w, int h, int b,
                           float fArray[])
    {
        super.setSamples(x, y, w, h, b, fArray);
        notifyChanged();
    }

    public void setSamples(int x, int y, int w, int h, int b,
                           double dArray[])
    {
        super.setSamples(x, y, w, h, b, dArray);
        notifyChanged();
    }

}
