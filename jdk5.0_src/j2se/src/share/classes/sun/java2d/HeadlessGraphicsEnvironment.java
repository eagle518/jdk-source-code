/*
 * @(#)HeadlessGraphicsEnvironment.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d;

import java.awt.GraphicsEnvironment;
import java.awt.GraphicsDevice;
import java.awt.Graphics2D;
import java.awt.HeadlessException;
import java.awt.image.BufferedImage;
import java.awt.Font;
import java.text.AttributedCharacterIterator;
import java.awt.print.PrinterJob;
import java.util.Map;
import java.util.Hashtable;
import java.util.Locale;
import java.util.Vector;
import java.util.StringTokenizer;
import java.util.ResourceBundle;
import java.util.MissingResourceException;
import java.io.IOException;
import java.io.FilenameFilter;
import java.io.File;
import java.util.NoSuchElementException;
import sun.awt.FontConfiguration;
import java.util.TreeMap;
import java.util.Set;
import java.awt.font.TextAttribute;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.BufferedInputStream;
import java.util.Properties;
import sun.io.CharacterEncoding;
import java.awt.Point;
import java.awt.Rectangle;

/**
 * Headless decorator implementation of a SunGraphicsEnvironment
 */

public class HeadlessGraphicsEnvironment extends GraphicsEnvironment
    implements FontSupport {
    
    private GraphicsEnvironment ge;
    private FontSupport fontSupport;
    
    public HeadlessGraphicsEnvironment(GraphicsEnvironment ge) {
        this.ge = ge;
        if (ge instanceof FontSupport) {
            fontSupport = (FontSupport)ge;
        }
    }

    public GraphicsDevice[] getScreenDevices()
        throws HeadlessException {
        throw new HeadlessException();
    }

    public GraphicsDevice getDefaultScreenDevice()
        throws HeadlessException {
        throw new HeadlessException();
    }

    public Point getCenterPoint() throws HeadlessException {
        throw new HeadlessException();
    }

    public Rectangle getMaximumWindowBounds() throws HeadlessException {
        throw new HeadlessException();
    }

    public Graphics2D createGraphics(BufferedImage img) {
        return ge.createGraphics(img); }

    public Font[] getAllFonts() { return ge.getAllFonts(); }

    public String[] getAvailableFontFamilyNames() {
        return ge.getAvailableFontFamilyNames(); }

    public String[] getAvailableFontFamilyNames(Locale l) {
        return ge.getAvailableFontFamilyNames(l); }

    public FontConfiguration getFontConfiguration() {
        if (fontSupport != null) {
            return fontSupport.getFontConfiguration();
        }
	return null;
    }

    /* Used by FontManager : internal API */
    public GraphicsEnvironment getSunGraphicsEnvironment() {
	return ge;
    }
}
