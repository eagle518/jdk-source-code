/*
 * @(#)WDesktopProperties.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;
import sun.awt.*;
import java.util.*;
import java.awt.*;
import java.beans.*;

/*
 * Class encapsulating Windows desktop properties.;
 * This class exposes Windows user configuration values
 * for things like:
 *	Window metrics
 *	Accessibility, display settings
 *	Animation effects
 *	Colors
 * 	Etc, etc etc.
 *
 * It's primary use is so that Windows specific Java code;
 * like the Windows Pluggable Look-and-Feel can better adapt
 * itself when running on a Windows platform.
 */
class WDesktopProperties {
    private static final DebugHelper	dbg = DebugHelper.create(WDesktopProperties.class);
    private static final String		PREFIX = "win.";
    private static final String		FILE_PREFIX = "awt.file.";
    private static final String		PROP_NAMES = "win.propNames";
    
    private long pData;
    private PropertyChangeSupport prChgSupport = new PropertyChangeSupport(this);

    static {
        initIDs();
    }

    private WToolkit wToolkit;

    private HashMap map = new HashMap();

    /**
     * Initialize JNI field and method IDs
     */
    private static native void initIDs();

    static boolean isWindowsProperty(String name) {
	return name.startsWith(PREFIX) || name.startsWith(FILE_PREFIX);
    }

    WDesktopProperties(WToolkit wToolkit) {
	this.wToolkit = wToolkit;
	init();
	getWindowsParameters();
    }

    protected native void finalize();

    public synchronized void addPropertyChangeListener(String name, PropertyChangeListener pcl) {
	if (pcl == null) {
	    return;
	}
	prChgSupport.addPropertyChangeListener(name, pcl);
    }

    /**
     * remove the specified property change listener for the named 
     * desktop property
     * If pcl is null, no exception is thrown and no action is performed.
     *
     */
    public synchronized void removePropertyChangeListener(String name, PropertyChangeListener pcl) {
	if (pcl == null) {
	    return;
	}
	prChgSupport.removePropertyChangeListener(name, pcl);
    }

    private native void init();

    /**
     * Returns property given a key name
     */
    synchronized Object getProperty(String key) {
	if ( key.equals(PROP_NAMES) ) {
	    return getKeyNames();
	}

	return map.get(key);
    }

    /*
     * Returns String[] containing available property names
     */
    private String [] getKeyNames() {
	Object	keys[] = map.keySet().toArray();
	String	sortedKeys[] = new String[keys.length];

	for ( int nkey = 0; nkey < keys.length; nkey++ ) {
	    sortedKeys[nkey] = keys[nkey].toString();
	}
	Arrays.sort(sortedKeys);
	return sortedKeys;
    }

    /*
     * Reads Win32 configuration information and
     * updates hashmap values
     */
    private native void getWindowsParameters();

    /*
     * Called from native code to set a boolean property
     */
    private synchronized void setBooleanProperty(String key, boolean value) {
	if (dbg.on) dbg.assertion( key != null );
	if (dbg.on) dbg.println(key + "=" + String.valueOf(value));
	map.put(key, new Boolean(value));
    }

    /*
     * Called from native code to set an integer property
     */
    private synchronized void setIntegerProperty(String key, int value) {
	if (dbg.on) dbg.assertion( key != null );
	if (dbg.on) dbg.println(key + "=" + String.valueOf(value));
	map.put(key, new Integer(value));
    }

    /*
     * Called from native code to set a string property
     */
    private synchronized void setStringProperty(String key, String value) {
	if (dbg.on) dbg.assertion( key != null );
	if (dbg.on) dbg.println(key + "=" + value);
	map.put(key, value);
    }

    /*
     * Called from native code to set a color property
     */
    private synchronized void setColorProperty(String key, int r, int g, int b) {
	if (dbg.on) dbg.assertion( key != null && r <= 255 && g <=255 && b <= 255 );
	Color color = new Color(r, g, b);
	if (dbg.on) dbg.println(key + "=" + color);
	map.put(key, color);
    }

    /*
     * Called from native code to set a font property
     */
    private synchronized void setFontProperty(String key, String name, int style, int size) {
	if (dbg.on) dbg.assertion( key != null && style <= (Font.BOLD|Font.ITALIC)  && size >= 0 );

	Font	font = new Font(name, style, size);
	if (dbg.on) dbg.println(key + "=" + font);
	map.put(key, font);

        String sizeKey = key + ".height";
        Integer iSize = new Integer(size);
	if (dbg.on) dbg.println(sizeKey + "=" + iSize);
	map.put(sizeKey, iSize);
    }

    /*
     * Called from native code to set a sound event property
     */
    private synchronized void setSoundProperty(String key, String winEventName) {
	if (dbg.on) dbg.assertion( key != null && winEventName != null );

	Runnable soundRunnable = new WinPlaySound(winEventName);
	if (dbg.on) dbg.println(key + "=" + soundRunnable);
	map.put(key, soundRunnable);
    }

    /*
     * Plays Windows sound event
     */
    private native void playWindowsSound(String winEventName);

    class WinPlaySound implements Runnable {
	String	winEventName;

	WinPlaySound(String winEventName) {
	    this.winEventName = winEventName;
	}

	public void run() {
	    WDesktopProperties.this.playWindowsSound(winEventName);
	}

	public String toString() {
	    return "WinPlaySound("+winEventName+")";
	}
    }

    /*
     * Called by WToolkit when Windows settings change-- we determine which
     * properties changed and fire change events for them
     */
    synchronized void firePropertyChanges() {
	EventQueue.invokeLater(new DiffPropertyChanges());
    }

    /*
     * We don't want property change listeners to block the
     * toolkit thread, so we use invokeLater to fire
     * property change events on the dispatch thread
     */
    class DiffPropertyChanges implements Runnable {
	public void run() {
	    diffPropertyChanges();
	}
    }

    /*
     * Determines which properties have changed and
     * fires change events for those values only
     */
    private synchronized void diffPropertyChanges() {	    
	// save the old properties
	HashMap oldmap = map;
	
	// load the changed properties into a new hashmap
	map = new HashMap();
	getWindowsParameters();
	
	// compare the old/new property values and fire events
	// for all the values that have changed
	String	keys[] = getKeyNames();
	for ( int nprop = 0; nprop < keys.length; nprop++ ) {
	    String name = keys[nprop];
	    Object	oldval = oldmap.get(name);
	    Object	newval = map.get(name);
	    if (((oldval == null) != (newval == null)) ||
		(oldval != null && !oldval.equals(newval))) {

		if (dbg.on) dbg.println("changed "+name+" from "+oldval+" to "+newval);
		wToolkit.clearDesktopProperty(name);
		prChgSupport.firePropertyChange(name, oldval, newval);
	    }
	}
    }
}
