/*
 * @(#)XAWTXSettings.java	1.4 04/01/21
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

 /* 
   * This code is ported to XAWT from MAWT based on awt_mgrsel.c 
   * and XSettings.java code written originally by Valeriy Ushakov 
   * Author : Bino George 
   */ 


package sun.awt.X11;

import java.util.*;
import java.awt.*;
import sun.awt.XSettings;
import sun.awt.*;
import java.util.logging.*;


 
class XAWTXSettings extends XSettings implements XMSelectionListener {

    private final XAtom xSettingsPropertyAtom = XAtom.get("_XSETTINGS_SETTINGS");

    private static Logger log = Logger.getLogger("sun.awt.X11.XAWTXSettings");

    /* The maximal length of the property data. */
    public static final long MAX_LENGTH = 1000000;

    XMSelection settings;

    public XAWTXSettings() {
        initXSettings();

    }

    void initXSettings() {
        if (log.isLoggable(Level.FINE)) log.fine("Initializing XAWT XSettings");
        settings = new XMSelection("_XSETTINGS");
        settings.addSelectionListener(this);
        updatePerScreenXSettings();
    }

    public void ownerDeath(int screen, XMSelection sel, long deadOwner) {
        if (log.isLoggable(Level.FINE)) log.fine("Owner " + deadOwner + " died for selection " + sel + " screen "+ screen);
    }

 
    public void ownerChanged(int screen, XMSelection sel, long newOwner, long data, long timestamp) { 
        if (log.isLoggable(Level.FINE)) log.fine("New Owner "+ newOwner + " for selection = " + sel + " screen " +screen );
    }
   
    public void selectionChanged(int screen, XMSelection sel, long owner , XPropertyEvent event) {
        log.fine("Selection changed on sel " + sel + " screen = " + screen + " owner = " + owner + " event = " + event);
        updateXSettings(screen,owner);
    }

    void updatePerScreenXSettings() {
        if (log.isLoggable(Level.FINE)) log.fine("Updating Per XSettings changes");

        /*
         * As toolkit cannot yet cope with per-screen desktop properties,
         * only report XSETTINGS changes on the default screen.  This
         * should be "good enough" for most cases.
         */

        synchronized (XToolkit.getAWTLock()) {
            long display = XToolkit.getDisplay();    
            int screen = (int) XlibWrapper.DefaultScreen(display);
            updateXSettings(screen);
        }
    }

    void updateXSettings(int screen) {
        long window = settings.getOwner(screen);
        updateXSettings(screen, window);
    }

    void updateXSettings(int screen, long owner) {
        if (log.isLoggable(Level.FINE)) log.fine("updateXSettings owner =" + owner);
        if (owner != 0 ) {

            try {
                WindowPropertyGetter getter = 
                    new WindowPropertyGetter(owner, xSettingsPropertyAtom, 0, MAX_LENGTH,
                            false, xSettingsPropertyAtom.getAtom() );
                try {
                    int status = getter.execute(XToolkit.IgnoreBadWindowHandler);

                    if (status != XlibWrapper.Success || getter.getData() == 0) {
                        if (log.isLoggable(Level.FINE)) log.fine("OH OH : getter failed  status = " + status );
                        return ;
                    }

                    long ptr = getter.getData();

                    if (log.isLoggable(Level.FINE)) log.fine("noItems = " + getter.getNumberOfItems());
                    byte array[] = Native.toBytes(ptr,getter.getNumberOfItems());
                    if (array != null) {
                        Map updatedSettings = update(array);
                        ((XToolkit)Toolkit.getDefaultToolkit()).parseXSettings(0,updatedSettings);
                    }
                } finally {
                    getter.dispose();
                }
            }
            catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    

}
