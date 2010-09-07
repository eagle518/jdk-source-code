/*
 * @(#)AppletContainer.java	1.26 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This code is located in the jnlp2 package - since it
 * is accessed directly from the Applet running in a sandbox it needs
 * to be outside the com.sun.javaws package
 */
package com.sun.jnlp;

import java.io.*;
import java.lang.reflect.*;
import java.applet.*;
import java.net.*;
import java.security.*;
import java.util.*;
import java.awt.BorderLayout;
import java.awt.Frame;
import java.awt.Image;
import java.awt.Dimension;
import java.awt.Insets;
import java.awt.Toolkit;
import sun.awt.image.URLImageSource;
import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JFrame;
import javax.swing.SwingUtilities;
import javax.swing.border.EtchedBorder;
import com.sun.javaws.Main;
import com.sun.javaws.exceptions.ExitException;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
/** Implentation of the Applet.AudioClip api. This code is heavily
 *  inspired (more or less copied) from sun/applet/AppletAudioClip.java
 */
final class AppletAudioClip implements AudioClip {
    
    // Constructor that we use for instantiating AudioClip objects.
    // Represents the constructor for either sun.audio.SunAudioClip (default) or
    // com.sun.media.sound.JavaSoundAudioClip (if the Java Sound extension is installed).
    private static Constructor acConstructor = null;
    
    // url that this AudioClip is based on
    private URL url = null;
    
    // the audio clip implementation
    private AudioClip audioClip = null;
    
    /** Used for forcing preloading of this class */
    public AppletAudioClip() {}
    
    /**
     * Constructs an AppletAudioClip from an URL.
     */
    public AppletAudioClip(URL url) {
        // store the url
        this.url = url;
        
        try {
            // create a stream from the url, and use it
            // in the clip.
            InputStream in = url.openStream();
            createAppletAudioClip(in);
            
        } catch (IOException e) {
            /* just quell it */
            
	    Trace.println("IOException creating AppletAudioClip" + e, TraceLevel.BASIC);
            
        }
    }
    
    private static Map audioClips = new HashMap();
    
    // This code is the same as in the AppletViewer
    static public synchronized  AudioClip get(URL url) {
        checkConnect(url);
        AudioClip clip = (AudioClip)audioClips.get(url);
        if (clip == null) {
            clip = new AppletAudioClip(url);
            audioClips.put(url, clip);
        }
        return clip;
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
            Trace.println("Initializing AudioClip constructor.", TraceLevel.BASIC);
            try {
                acConstructor = (Constructor) AccessController.doPrivileged( new PrivilegedExceptionAction() {
                            public Object run() throws NoSuchMethodException, SecurityException, ClassNotFoundException {
                                Class acClass  = null;
                                try {
                                    // attempt to load the Java Sound extension class JavaSoundAudioClip
                                    acClass = Class.forName("com.sun.media.sound.JavaSoundAudioClip",
                                                            true,
                                                            ClassLoader.getSystemClassLoader()); // may throw ClassNotFoundException
                                    
                                    Trace.println("Loaded JavaSoundAudioClip", TraceLevel.BASIC);
                                } catch (ClassNotFoundException e) {
                                    acClass = Class.forName("sun.audio.SunAudioClip", true, null);      // may throw ClassNotFoundException
                                    Trace.println("Loaded SunAudioClip", TraceLevel.BASIC);
                                }
                                
                                Class[] parms = new Class[1];
                                parms[0] = Class.forName("java.io.InputStream");                                // may throw ClassNotFoundException
                                return acClass.getConstructor(parms);   // may throw NoSuchMethodException or SecurityException
                            }
                        } );
                
            } catch (PrivilegedActionException e) {
                Trace.println("Got a PrivilegedActionException: " + e.getException(), TraceLevel.BASIC);
                
                // e.getException() may be a NoSuchMethodException, SecurityException, or ClassNotFoundException.
                // however, we throw an IOException to avoid changing the interfaces....
                throw new IOException("Failed to get AudioClip constructor: " + e.getException());
            }
        } // if not initialized
        
        
        // Now instantiate the AudioClip object using the constructor we discovered above.
        try {
            Object[] args = new Object[] {in};
            audioClip = (AudioClip)acConstructor.newInstance(args);     // may throw InstantiationException,
            // IllegalAccessException,
            // IllegalArgumentException,
            // InvocationTargetException
        } catch (Exception e3) {
            // no matter what happened, we throw an IOException to avoid changing the interfaces....
            throw new IOException("Failed to construct the AudioClip: " + e3);
        }
    }
    
    /** Check if the security policy allows access to this URL */
    private static void checkConnect(URL url) {
        SecurityManager security = System.getSecurityManager();
        if (security != null) {
            try {
                java.security.Permission perm =
                    url.openConnection().getPermission();
                if (perm != null)
                    security.checkPermission(perm);
                else
                    security.checkConnect(url.getHost(), url.getPort());
            } catch (java.io.IOException ioe) {
                security.checkConnect(url.getHost(), url.getPort());
            }
        }
    }
    
    //
    // java.applet.AudioClip interface
    //
    public synchronized void play() {
        if (audioClip != null) audioClip.play();
    }
    
    public synchronized void loop() {
        if (audioClip != null) audioClip.loop();
    }
    
    public synchronized void stop() {
        if (audioClip != null) audioClip.stop();
    }
};


/** Cache of all downloaded images. The AppletViewer does this,
 *   so it is probably a good idea
 *   (AppletViewer caches images with weak-references
 *    That might give better performance on some applets)
 */
class ImageCache {
    private static Map images = null;
    
    static synchronized Image getImage(URL url) {
        Image image = (Image)images.get(url);
        if (image == null) {
            image = Toolkit.getDefaultToolkit().createImage(new URLImageSource(url));
            images.put(url, image);
        }
        return image;
    }
    
    static public void initialize() {
        images = new HashMap();
    }
}

/**
 *   Implements a simple applet container.
 *
 */
public final class AppletContainer extends JPanel {
    
    // Callback stub
    final AppletContainerCallback callback;
    
    // Applet parameters
    final Applet     applet;
    final String     appletName;
    final URL        documentBase;
    final URL        codeBase;
    final Properties parameters;
    final boolean    isActive[] = { false };
    
    // Size of applet
    int   appletWidth;
    int   appletHeight;
    
    // GUI
    final JLabel statusLabel = new JLabel("");
    
    static class LoadImageAction implements PrivilegedAction {
        URL _url;
        public LoadImageAction(URL url) { _url = url; }
        public Object run() { return ImageCache.getImage(_url); }
    }
    // Dummy object to force preloading
    static PrivilegedAction loadImageActionDummy = new LoadImageAction(null);
    
    /** Concrete implementation of the java.applet.AppletContext class */
    class AppletContainerContext implements AppletContext {
        /** There is always only one applet runing, so just return
         *  this one if the name matches
         */
        public Applet getApplet(String name) {
            return name.equals(appletName) ? applet : null;
        }
        
        /** Return an enumeration with one element */
        public Enumeration getApplets() {
            Vector v = new Vector();
            v.add(applet);
            return v.elements();
        }
        
        public AudioClip getAudioClip(URL url) {
            return AppletAudioClip.get(url);
        }
        
        public Image getImage(URL url) {
            // This has to be in a doPrivileged, since it invoke methods in the sun.* package
            LoadImageAction lia = new LoadImageAction(url);
            return (Image)AccessController.doPrivileged(lia);
        }
        
        public void showDocument(final URL url) {
            // Execute command in separate thread. This could be the event
            // dispatcher thread
            AccessController.doPrivileged( new PrivilegedAction() {
                        public Object run() {
                            Thread t = new Thread() {
                                public void run() {
                                    callback.showDocument(url);
                                };
                            };
                            t.start();
                            return null;
                        }
                    });
        }
        
        public void showDocument(URL url, String target) {
            showDocument(url);
        }
        
        public void showStatus(String status) {
            statusLabel.setText(status);
        }
        

	/* added in 1.4 in AppletContext - here so can compile with 1.4 */
        public void setStream(String key, InputStream stream) {
	}

	public InputStream getStream(String key) {
		return null;
	}

	public Iterator getStreamKeys() {
		return null;
	}

    };
    
    /** Inner class that provides a concrete implementation
     *  of the java.applet.AppletStub class
     */
    final class AppletContainerStub implements AppletStub {
        AppletContext context;
        
        AppletContainerStub(AppletContext context) {
            this.context = context;
        }
        
        public void appletResize(int width, int height) {
            resizeApplet(width, height);
        }
        
        public AppletContext getAppletContext() {
            return context;
        }
        
        public URL getCodeBase() {
            return codeBase;
        }
        
        public URL getDocumentBase() {
            return documentBase;
        }
        
        public String getParameter(String name) {
            return parameters.getProperty(name);
        }
        
        public boolean isActive() {
            // The array is a trick to get around the final restriction
            // on inner classes
            return isActive[0];
        }
    };
    
    
    /** Contruct the applet container object, and sets it size
     requirements
     */
    public AppletContainer(AppletContainerCallback callback,
                           Applet applet,
                           String name,
                           URL documentBase,
                           URL codeBase,
                           int width,
                           int height,
                           Properties parameters) {
        super();
        
        // Setup parameters
        this.callback     = callback;
        this.applet       = applet;
        this.appletName   = name;
        this.documentBase = documentBase;
        this.codeBase     = codeBase;
        this.parameters   = parameters;
        this.isActive[0]  = false;
        this.appletWidth  = width;
        this.appletHeight = height;
        
        // Attach stub to applet
        AppletContext ac = new AppletContainerContext();
        AppletStub as = new AppletContainerStub(ac);
        applet.setStub(as);
        
        // Setup gui
        statusLabel.setBorder(new EtchedBorder());
        statusLabel.setText("Loading...");
        setLayout(new BorderLayout());
        add("Center", applet);
        add("South" , statusLabel);
        
        Dimension d = new Dimension(appletWidth,
                                    appletHeight + (int)statusLabel.getPreferredSize().getHeight());
        setPreferredSize(d);
    }
    
    /** returns the applet associated with this container */
    public Applet getApplet() { return applet; }
    
    public void setStatus(String msg) {
        statusLabel.setText(msg);
    }
    
    
    public void resizeApplet(int width, int height) {
        // Ignore illegal arguments
        if (width < 0 || height < 0) return;
        
        // Calculate the delta for the container
        int deltaWidth  = width  - appletWidth;
        int deltaHeight = height - appletHeight;
        
        // Apply the delta to the container
        Dimension curDim = getSize();
        Dimension newDim = new Dimension((int)curDim.getWidth()  + deltaWidth,
                                             (int)curDim.getHeight() + deltaHeight);
        setSize(newDim);
        
        // Tell frame to resize
        callback.relativeResize(new Dimension(deltaWidth, deltaHeight));
        
        // Remember new size
        appletWidth = width;
        appletHeight = height;
    }
    
    /** Computes size of frame so the applet with have the specified space.
     *  This method must be called after pack() or show()
     */
    public Dimension getPreferredFrameSize(Frame f) {
        Insets insets = f.getInsets();
        int w = appletWidth  + (insets.left + insets.right);
        int h = appletHeight + statusLabel.getHeight() + (insets.top + insets.bottom);
        return new Dimension(w, h);
    }
    
    
    public void startApplet() {
        // Initialize ImageMap. This forces preloading, which simplifies security
        ImageCache.initialize();
        new AppletAudioClip(); // Preload class
        
        
        // Start a new thread to execute applet.
        new Thread() {
            public void run() {
                try {
                    setStatus("Initializing Applet");
                    applet.init();
                    
                    try {
                        // Mark applet as active
                        isActive[0] = true;
                        applet.start();
                        setStatus("Applet running...");
                    } catch(Throwable e) {
                        setStatus("Failed to start Applet: " + e.toString());
                        // Print exception to std out for debugging (this goes to the console)
                        e.printStackTrace(System.out); // This is suppose to be here
                        isActive[0] = false;
                    }
                } catch(Throwable e) {
                    setStatus("Failed to initialize: " + e.toString());
                    // Print exception to std out for debugging (this goes to the console)
                    e.printStackTrace(System.out); // This is suppose to be here
                    
                }
            }
        }.start();
    }

 
   // This method will be executed whenever the window which is running
  // the applet is closed
   
    public void stopApplet()
    {
        applet.stop();
        applet.destroy();
        try {
            Main.systemExit(0);
        } catch (ExitException ee) { 
            Trace.println("systemExit: "+ee, TraceLevel.BASIC);
            Trace.ignoredException(ee);
        }
    } 
    
   /* Quick and dirty test method. Debug code only */
    static void showApplet(AppletContainerCallback callback,
                           final Applet applet,
                           String name,
                           URL documentBase,
                           URL codeBase,
                           int width,
                           int height,
                           Properties parameters) {
        JFrame frame = new JFrame("Applet Window");
        
        final AppletContainer container = new AppletContainer(
            callback,
            applet,
            name,
            documentBase,
            codeBase,
            width,
            height,
            parameters);
        
        frame.getContentPane().setLayout(new BorderLayout());
        frame.getContentPane().add("Center", container);
        frame.pack();
        frame.setVisible(true);
        
        SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
                        try {
                            container.setStatus("Initializing Applet");
                            applet.init();
                            applet.start();
                            container.setStatus("Applet Running");
                        } catch(Throwable e) {
                            container.setStatus("Failed to start Applet");
                        }
                    }
                });
    }

}



