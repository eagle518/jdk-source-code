/*
 * @(#)Applet2AudioClipFactory.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

/*
 * Helper class that create applet audio clip.
 *
 * @version 	1.1
 * @author	Stanley Man-Kit Ho
 */

import java.awt.Toolkit;
import java.applet.AudioClip;
import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.lang.reflect.Constructor;
import java.net.URL;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedActionException;
import java.security.PrivilegedExceptionAction;


public final class Applet2AudioClipFactory
{
    /**
     * Create the audio clip.
     */
    public static AudioClip createAudioClip(final URL url) 
    {
	AudioClip clip = (AudioClip) AccessController.doPrivileged(
	    new PrivilegedAction() 
	    {
		public Object run() 
		{
		    // The following code is to resolve JDK 1.1
		    // compatibility with Internet Explorer.
		    // 
		    // In IE, resources are always searched in 
		    // the archives first before searching the
		    // codebase, but this is not the case in
		    // NS or appletviewer. 
		    //
		    // To enhance compatibility, Java Plug-in
		    // will search the archives first before
		    // the codebase.
		    //
		    // The following step has been taken:
		    //
		    // We check if the url is from the base URL
		    // of the classloader. If so, we try to 
		    // load the resources using getResourcesAsStream
		    // with the following sequences:
		    //
		    // 1. Load resources from archives
		    // 2. Load resources from codebase
		    //
		    // [stanleyh]
		    try
		    {
			Thread t = Thread.currentThread();
			ClassLoader cl = t.getContextClassLoader();

			if (cl != null && cl instanceof Applet2ClassLoader)
			{
			    Applet2ClassLoader pcl = (Applet2ClassLoader) cl;

			    // Determine classloader URL
			    String clsLoaderURLString = pcl.getBaseURL().toString();

			    // Separate path and resource name - resource name 
			    // may contain directory structure
			    //
			    String audioURLString = url.toString();
			    int i = audioURLString.indexOf(clsLoaderURLString);

			    if (i == 0) 
			    {
				String clipName;

				// This is to fix problem like 
				//
				// http://fdl.msn.com/zone/games/SAC/DIMI//sounds/click2.au
				//
				// so the extra '/' is eliminated
				//
				if (audioURLString.charAt(clsLoaderURLString.length()) == '/')
				    clipName = audioURLString.substring(clsLoaderURLString.length() + 1);
				else
				    clipName = audioURLString.substring(clsLoaderURLString.length());

				// Get resources from classloader as stream
				InputStream is = pcl.getResourceAsStream(clipName);

				// If stream exists
				if (is != null)
				{
				    BufferedInputStream bis = new BufferedInputStream(is);
				    ByteArrayOutputStream bos = new ByteArrayOutputStream();
				    byte[] buffer = new byte[8192];

				    int byteRead = 0;

				    // Read the stream until it is EOF
				    while ((byteRead = bis.read(buffer, 0, 8192)) != -1)
					bos.write(buffer, 0, byteRead);
	    
				    // Close input stream
				    bis.close();

				    // Convert to byte array
				    byte[] data = bos.toByteArray();

    				    // Return audio clip only if data length is not zero
				    if (data != null && data.length > 0)  
					return new AppletAudioClip(data);
				}
			    }
			}
		    
			// The base URL doesn't match or no
			// context classloader, so load
			// the resources from URL directly.
			//
			return new AppletAudioClip(url);
		    }
		    catch (Exception e)
		    {
			e.printStackTrace();

			// If we have any exception or unable to load the 
			// resource, return null;
			//
			return null;
		    }

		}
	    });

	if (clip != null)
	    return new PluginAudioClip(clip);
	else
	    return null;
    }
}


class PluginAudioClip implements AudioClip
{
    private AudioClip clip = null;

    public PluginAudioClip(AudioClip clip)
    {
	this.clip = clip;
    }

    public void loop()
    {
	if (clip != null)
	    clip.loop();
    }

    public void play()
    {
	if (clip != null)
	    clip.play();
    }

    public void stop()
    {
	if (clip != null)
	    clip.stop();
    }

    public void finalize()
    {
	// Make sure audio clip is stopped before GC,
	// this is to avoid locking up the audio system.
	stop();
	
	clip = null;
    }
}


class AppletAudioClip implements AudioClip 
{
    // Constructor that we use for instantiating AudioClip objects.
    // Represents the constructor for either sun.audio.SunAudioClip (default) or
    // com.sun.media.sound.JavaSoundAudioClip (if the Java Sound extension is installed).
    private static Constructor acConstructor = null;	

    // url that this AudioClip is based on
    private URL url = null;	

    // the audio clip implementation
    private AudioClip audioClip = null;

    boolean DEBUG = false /*true*/;

    /**
     * Constructs an AppletAudioClip from an URL.
     */
    AppletAudioClip(URL url) {
	
	// store the url
	this.url = url;
	
	try {
	    // create a stream from the url, and use it
	    // in the clip.
	    InputStream in = url.openStream(); 
	    createAppletAudioClip(in);
	    
	} catch (IOException e) { 
		/* just quell it */ 
	    if (DEBUG) {
		System.err.println("IOException creating AppletAudioClip" + e);
	    }
	}
    }

    /** 
     * For constructing directly from Jar entries, or any other
     * raw Audio data. Note that the data provided must include the format
     * header.
     */
    AppletAudioClip(byte [] data) {
	
	try {
	    
	    // construct a stream from the byte array
	    InputStream in = new ByteArrayInputStream(data);
	    
	    createAppletAudioClip(in);
	    
	} catch (IOException e) { 
		/* just quell it */ 
	    if (DEBUG) {
		System.err.println("IOException creating AppletAudioClip " + e);
	    }
	}
    }


    /*
     * Does the real work of creating an AppletAudioClip from an InputStream.
     * This function is used by both constructors.
     */
    void createAppletAudioClip(InputStream in) throws IOException {

	// If we haven't initialized yet, we need to find the AudioClip constructor using reflection.
	// We'll use com.sun.media.sound.JavaSoundAudioClip to implement AudioClip if the Java Sound
	// extension is installed.  Otherwise, we use sun.audio.SunAudioClip.


	if (acConstructor == null) {

	    if (DEBUG) System.out.println("Initializing AudioClip constructor.");

	    try {

		acConstructor = (Constructor) AccessController.doPrivileged( new PrivilegedExceptionAction() {

		    public Object run() throws NoSuchMethodException, SecurityException, ClassNotFoundException {
			    
			Class acClass  = null;
			
			try {		
			
			    // attempt to load the Java Sound extension class JavaSoundAudioClip

			    acClass = Class.forName("com.sun.media.sound.JavaSoundAudioClip", 
						    true, 
						    ClassLoader.getSystemClassLoader()); // may throw ClassNotFoundException

			    if (DEBUG) System.out.println("Loaded JavaSoundAudioClip");

			} catch (ClassNotFoundException e) {

			    acClass = Class.forName("sun.audio.SunAudioClip", true, null); 	// may throw ClassNotFoundException

			    if (DEBUG) System.out.println("Loaded SunAudioClip");
			}

			Class[] parms = new Class[1];
			parms[0] = Class.forName("java.io.InputStream");				// may throw ClassNotFoundException
			return acClass.getConstructor(parms);	// may throw NoSuchMethodException or SecurityException
		    }
		} );
						    
	    } catch (PrivilegedActionException e) {

		if (DEBUG) System.out.println("Got a PrivilegedActionException: " + e.getException());

		// e.getException() may be a NoSuchMethodException, SecurityException, or ClassNotFoundException.
		// however, we throw an IOException to avoid changing the interfaces....

		throw new IOException("Failed to get AudioClip constructor: " + e.getException());
	    }
		
	} // if not initialized


	// Now instantiate the AudioClip object using the constructor we discovered above.

	try {
	    Object[] args = new Object[] {in};
	    audioClip = (AudioClip)acConstructor.newInstance(args);	// may throw InstantiationException,
	} catch (Exception e3) {
	    // no matter what happened, we throw an IOException to avoid changing the interfaces....
	    throw new IOException("Failed to construct the AudioClip: " + e3);
	}

    }


    public synchronized void play() {
	if (audioClip != null)
	    audioClip.play();
    }

    
    public synchronized void loop() {
	if (audioClip != null)
	    audioClip.loop();
    }

    public synchronized void stop() {
	if (audioClip != null)
	    audioClip.stop();
    }
}



