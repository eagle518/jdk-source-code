/*
 * @(#)ColorUtil.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.util;

import java.awt.Color;
import java.util.StringTokenizer;
import com.sun.deploy.util.Trace;

public class ColorUtil {

    /*
     * Create color
     * @param html_tag - name of the html tag to use in tracing message
     * @param str - string representation of color.  It should consist three digits
     *              separated by commas, or one of standard colors.
     *
     * Returns color or null(if number of components for RGB color was incorrect,
     * or if standard color was spelled incorrectly).
     */
    public static Color createColor(String html_tag, String str) {
        if (str != null && str.indexOf(",") != -1) {
            // This is the RGB format.  Tokenize the string.
            StringTokenizer st = new StringTokenizer(str, ",");
            if (st.countTokens()==3) {
                // We've got three components for the color.
                int i=0;
                int red=0, green=0, blue=0;
                while (st.hasMoreTokens()) {
                    String token = (String)st.nextElement();
                    switch (i) {
                    case 0: {
                        if (!token.trim().equals(""))
                            red = new Integer(token.trim()).intValue();
                        break;
                    }
                    case 1: {
                        if (! token.trim().equals(""))
                            green = new Integer(token.trim()).intValue();
                        break;
                    }
                    case 2: {
                        if(! token.trim().equals(""))
                            blue = new Integer(token.trim()).intValue();
                        break;
                    }
                    }
                    i++;
                }
                return new Color(red, green, blue);
            } else {
                Trace.msgPrintln("applet_viewer.color_tag", new Object[] {html_tag});
                return null;
            }
        } else if (str != null) {
            // Check & decode if the color is in hexadecimal color format (i.e. #808000)
            try {
                return Color.decode(str);
            } catch (NumberFormatException e) {
                // ignore exception
            }
            
            // This is a string representation of color
            if (str.equalsIgnoreCase("red"))
                return Color.red;
            if (str.equalsIgnoreCase("yellow"))
                return Color.yellow;
            if (str.equalsIgnoreCase("black"))
                return Color.black;
            if (str.equalsIgnoreCase("blue"))
                return Color.blue;
            if (str.equalsIgnoreCase("cyan")
                || str.equalsIgnoreCase("aqua"))
                return Color.cyan;
            if (str.equalsIgnoreCase("darkGray"))
                return Color.darkGray;
            if (str.equalsIgnoreCase("gray"))
                return Color.gray;
            if (str.equalsIgnoreCase("lightGray")
                || str.equalsIgnoreCase("silver"))
                return Color.lightGray;
            // green is Java std #00ff00 not w3 HTML std.
            // w3 HTML std - lime is #00ff00 & green is #008000
            if (str.equalsIgnoreCase("green")
                || str.equalsIgnoreCase("lime"))
                return Color.green;
            if (str.equalsIgnoreCase("magenta")
                || str.equalsIgnoreCase("fuchsia"))
                return Color.magenta;
            if (str.equalsIgnoreCase("orange"))
                return Color.orange;
            if (str.equalsIgnoreCase("pink"))
                return Color.pink;
            if (str.equalsIgnoreCase("white"))
                return Color.white;
            if (str.equalsIgnoreCase("maroon"))
                return new Color(128, 0, 0);
            if (str.equalsIgnoreCase("purple"))
                return new Color(128, 0, 128);
            if (str.equalsIgnoreCase("navy"))
                return new Color(0, 0, 128);
            if (str.equalsIgnoreCase("teal"))
                return new Color(0, 128, 128);
            if (str.equalsIgnoreCase("olive"))
                return new Color(128, 128, 0);
        }
        
        /*
         * Misspelling?
         */
        return null;
    }

    public static int createColorRGB (String html_tag, String str) {

	Color c = createColor(html_tag, str);

	if (c != null) {
	    return c.getRGB();
	}

	return 0;
    }

}
