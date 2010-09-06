/*
 * @(#)IconManager.java	1.4 04/04/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jconsole.inspector;


// java import
import java.awt.*;
import javax.swing.*;
import java.net.*;
import java.util.*;
import java.io.*;
//
import sun.tools.jconsole.JConsole;
public class IconManager {
    public static Icon MBEANTREE_ROOT = 
	getSmallIcon(getImage("mbeantree_root.gif")); 
    public static Icon STANDARDMBEAN = 
	getSmallIcon(getImage("standardmbean.gif"));
    public static Icon OPENMBEAN = getSmallIcon(getImage("openmbean.gif"));
    public static Icon MODELMBEAN = getSmallIcon(getImage("modelmbean.gif"));
    public static Icon MBEANSERVERDELEGATE = 
	getSmallIcon(getImage("mbeanserverdelegate.gif"));
    public static Icon DEFAULT_XOBJECT = getSmallIcon(getImage("xobject.gif"));
    private static ImageIcon getImage(String img) {
	return new ImageIcon(JConsole.class.getResource("resources/" + img));
    }
    
    private static ImageIcon getSmallIcon(ImageIcon icon) {
	return 
	    new ImageIcon(icon.getImage().
			  getScaledInstance(16,
					    16,
					    Image.SCALE_SMOOTH));
    }
}
