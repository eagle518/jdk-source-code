/*
 * @(#)WEmbeddedFrame.java	1.27 04/05/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import sun.awt.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.peer.ComponentPeer;
import java.util.*;
import java.awt.color.*;
import java.awt.image.*;
import sun.awt.image.ByteInterleavedRaster;


public class WEmbeddedFrame extends EmbeddedFrame {

    static {
        initIDs();
    }

    private long handle;

    private int bandWidth = 0;
    private int bandHeight = 0;
    private int imgWid = 0;
    private int imgHgt = 0;

    private static final int MAX_BAND_SIZE = (1024*30); 

    public WEmbeddedFrame() {
        this((long)0);
    }
    
    /**
     * @deprecated This constructor will be removed in 1.5
     */
    @Deprecated
    public WEmbeddedFrame(int handle) {
        this((long)handle);
    }

    public WEmbeddedFrame(long handle) {
        this.handle = handle;
        WToolkit toolkit = (WToolkit)Toolkit.getDefaultToolkit();
        setPeer(toolkit.createEmbeddedFrame(this));
        show();
    }

    /*
     * Print the embedded frame and its children using the specified HDC.
     */

    void print(int hdc) {
	BufferedImage bandImage = null;
 
	int xscale = 1;
	int yscale = 1;

	/* Is this is either a printer DC or an enhanced meta file DC ?
	 * Mozilla passes in a printer DC, IE passes plug-in a DC for an
	 * enhanced meta file. Its possible we may be passed to a memory
	 * DC. If we here create a larger image, draw in to it and have
	 * that memory DC then lose the image resolution only to scale it
	 * back up again when sending to a printer it will look really bad.
	 * So, is this is either a printer DC or an enhanced meta file DC ?
	 * Scale only if it is. Use a 4x scale factor, partly since for
	 * an enhanced meta file we don't know anything about the
	 * real resolution of the destination.
	 * 
	 * For a printer DC we could probably derive the scale factor to use
	 * by querying LOGPIXELSX/Y, and dividing that by the screen
	 * resolution (typically 96 dpi or 120 dpi) but that would typically
	 * make for even bigger output for marginal extra quality.
	 * But for enhanced meta file we don't know anything about the
	 * real resolution of the destination so 
	 */
	if (isPrinterDC(hdc)) {
	    xscale = 4;
	    yscale = 4;
	}

	int frameHeight = getHeight();
	if (bandImage == null) {
	    bandWidth = getWidth();
	    if (bandWidth % 4 != 0) {
		bandWidth += (4 - (bandWidth % 4));
	    }
	    if (bandWidth <= 0) {
		return;
	    }

	    bandHeight = Math.min(MAX_BAND_SIZE/bandWidth, frameHeight);

	    imgWid = (int)(bandWidth * xscale);
	    imgHgt = (int)(bandHeight * yscale);
	    bandImage = new BufferedImage(imgWid, imgHgt,
					  BufferedImage.TYPE_3BYTE_BGR);
	}

	Graphics clearGraphics = bandImage.getGraphics();
	clearGraphics.setColor(Color.white);
	Graphics2D g2d = (Graphics2D)bandImage.getGraphics();
	g2d.translate(0, imgHgt);
	g2d.scale(xscale, -yscale);

	ByteInterleavedRaster ras = (ByteInterleavedRaster)bandImage.getRaster();
	byte[] data = ras.getDataStorage();

	for (int bandTop = 0; bandTop < frameHeight; bandTop += bandHeight) {
	    clearGraphics.fillRect(0, 0, bandWidth, bandHeight);

	    printComponents(g2d);
	    int imageOffset =0;
	    int currBandHeight = bandHeight;
	    int currImgHeight = imgHgt;
	    if ((bandTop+bandHeight) > frameHeight) {
		// last band
		currBandHeight = frameHeight - bandTop;
		currImgHeight = (int)(currBandHeight*yscale);

		// multiply by 3 because the image is a 3 byte BGR
		imageOffset = imgWid*(imgHgt-currImgHeight)*3;
	    }
	
	    printBand((long)hdc, data, imageOffset,
		      0, 0, imgWid, currImgHeight,
		      0, bandTop, bandWidth, currBandHeight);
	    g2d.translate(0, -bandHeight);
	}
    }

    protected native boolean isPrinterDC(long hdc);
  
    protected native void printBand(long hdc, byte[] data, int offset,
				    int sx, int sy, int swidth, int sheight,
				    int dx, int dy, int dwidth, int dheight);

    /**
     * Initialize JNI field IDs
     */
    private static native void initIDs();

    public void synthesizeWindowActivation(boolean b) {
        synthesizeWmActivate(handle, b);
    }
    private static native void synthesizeWmActivate(long handle, boolean b);
}
