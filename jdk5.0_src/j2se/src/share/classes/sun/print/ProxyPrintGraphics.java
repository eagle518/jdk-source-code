/*
 * @(#)ProxyPrintGraphics.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)ProxyPrintGraphics.java	1.6 03/12/19
 *
 * Copyright 1998, 1999, 2000 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the proprietary information of Sun Microsystems, Inc.  
 * Use is subject to license terms.
 * 
 */

package sun.print;

import java.awt.Graphics;
import java.awt.PrintGraphics;
import java.awt.PrintJob;

/**
 * A subclass of Graphics that can be printed to. The
 * graphics calls are forwared to another Graphics instance
 * that does the actual rendering.
 */

public class ProxyPrintGraphics extends ProxyGraphics
			        implements PrintGraphics {

    private PrintJob printJob;

    public ProxyPrintGraphics(Graphics graphics, PrintJob thePrintJob) {
	super(graphics);
	printJob = thePrintJob;
    }

    /**
     * Returns the PrintJob object from which this PrintGraphics 
     * object originated.
     */
    public PrintJob getPrintJob() {
	return printJob;
    }

   /**
     * Creates a new <code>Graphics</code> object that is 
     * a copy of this <code>Graphics</code> object.
     * @return     a new graphics context that is a copy of 
     *                       this graphics context.
     */
    public Graphics create() {
	return new ProxyPrintGraphics(getGraphics().create(), printJob);
    }


    /**
     * Creates a new <code>Graphics</code> object based on this 
     * <code>Graphics</code> object, but with a new translation and
     * clip area.
     * Refer to
     * {@link sun.print.ProxyGraphics#createGraphics}
     * for a complete description of this method.
     * <p>
     * @param      x   the <i>x</i> coordinate.
     * @param      y   the <i>y</i> coordinate.
     * @param      width   the width of the clipping rectangle.
     * @param      height   the height of the clipping rectangle.
     * @return     a new graphics context.
     * @see        java.awt.Graphics#translate
     * @see        java.awt.Graphics#clipRect
     */
    public Graphics create(int x, int y, int width, int height) {
        Graphics g = getGraphics().create(x, y, width, height);
        return new ProxyPrintGraphics(g, printJob);
    }

    public Graphics getGraphics() {
        return super.getGraphics();
    }


   /* Spec implies dispose() should flush the page, but the implementation
    * has in fact always done this on the getGraphics() call, thereby
    * ensuring that multiple pages are cannot be rendered simultaneously.
    * We will preserve that behaviour and there is consqeuently no need
    * to take any action in this dispose method.
    */
    public void dispose() {
     super.dispose();
    }
            
}
