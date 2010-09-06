/*
 * @(#)SimpleInputDeviceProvider.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.util.Vector;

import javax.sound.sampled.Mixer;
import javax.sound.sampled.spi.MixerProvider;


/**
 * Simple input device provider.  
 *
 * @version 1.12 03/12/19
 * @author Kara Kytle
 */
public class SimpleInputDeviceProvider extends MixerProvider {


    // STATIC VARIABLES
			
	
    /**
     * Set of info objects for all simple input devices on the system.
     */
    private static InputDeviceInfo[] infos;
	
    /**
     * Set of all simple input devices on the system.
     */
    private static SimpleInputDevice[] devices;

	

    // STATIC

    /**
     * Create objects representing all simple input devices on the system.
     */
    static {

	if (Printer.trace) Printer.trace("SimpleInputDeviceProvider: static");
		
	// initialize 
	Platform.initialize();
		
	// get the number of input devices
	int numDevices = 0;
	
	// only want to provide devices if we do not have DirectAudioDevices
	if (!Platform.isDirectAudioEnabled()) {
	    numDevices = nGetNumDevices();
	}

	// initialize the arrays
	infos = new InputDeviceInfo[numDevices];
	devices = new SimpleInputDevice[numDevices];

	// fill in the info objects now.
	// we'll fill in the device objects as they're requested.

	String name;
	String vendor;
	String description; 
	String version;

	for (int i = 0; i < infos.length; i++) {
			
	    name = nGetName(i);
	    vendor = nGetVendor(i);
	    description = nGetDescription(i);
	    version = nGetVersion(i);

	    infos[i] = new InputDeviceInfo(name, vendor, description, version, i, SimpleInputDeviceProvider.class);
	}

	if (Printer.trace) Printer.trace("SimpleInputDeviceProvider: static: found numDevices: " + numDevices);
    }


    // CONSTRUCTOR


    /**
     * Required public no-arg constructor.
     */ 
    public SimpleInputDeviceProvider() {
	//if (Printer.trace) Printer.trace("SimpleInputDeviceProvider: constructor");
    }


    public Mixer.Info[] getMixerInfo() {

	Mixer.Info[] localArray = new Mixer.Info[infos.length];
	if (infos.length > 0) {
	    System.arraycopy(infos, 0, localArray, 0, infos.length);
	}
	return localArray;
    }	


    public Mixer getMixer(Mixer.Info info) {

	for (int i = 0; i < infos.length; i++) {

	    if (info == infos[i]) {
		return getDevice(infos[i]);
	    } 
	}

	throw new IllegalArgumentException("Mixer " + info.toString() + " not supported by this provider.");
    }


    private Mixer getDevice(InputDeviceInfo info) {

	int index = info.getIndex();

	if (devices[index] == null) {
	    devices[index] = new SimpleInputDevice(info);
	}

	return devices[index];
    }



    // INNER CLASSES


    /**
     * Info class for SimpleInputDevices.  Adds an index value for
     * making native references to a particular device and a the
     * provider's Class to keep the provider class from being 
     * unloaded.  Otherwise, at least on JDK1.1.7 and 1.1.8,
     * the provider class can be unloaded.  Then, then the provider
     * is next invoked, the static block is executed again and a new
     * instance of the device object is created.  Even though the 
     * previous instance may still exist and be open / in use / etc.,
     * the new instance will not reflect that state.... 
     */
    static class InputDeviceInfo extends Mixer.Info {

	private int index;
	private Class providerClass;

	private InputDeviceInfo(String name, String vendor, String description, String version, int index, Class providerClass) {

	    super(name, vendor, description, version);
	    this.index = index;
	    this.providerClass = providerClass;
	}


	int getIndex() {			
	    return index;
	}		

    } // class InputDeviceInfo


    // NATIVE METHODS  

    private static native int nGetNumDevices();
    private static native String nGetName(int index);
    private static native String nGetVendor(int index);
    private static native String nGetDescription(int index);
    private static native String nGetVersion(int index);
}





