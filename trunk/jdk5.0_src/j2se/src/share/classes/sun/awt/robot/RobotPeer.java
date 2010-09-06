/*
 * @(#)RobotPeer.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.robot;

import java.awt.*;

/**
 * An internal interface optionally implemented by Toolkit implementations.
 *
 * RobotPeer defines an interface whereby toolkits support automated testing
 * by allowing native input events to be generated from Java code.
 *
 * @version 	@(#)RobotPeer.java	1.6 03/12/19
 * @author 	Robi Khan
 */
public interface RobotPeer
{
    public void mouseMove(int x, int y);
    public void mousePress( long buttons );
    public void mouseRelease( long buttons );

    public void keyPress( int keycode );
    public void keyRelease( int keycode );

    public int getRGBPixel(int x, int y);
    public int [] getRGBPixels(Rectangle bounds);
}
