/*
 * @(#)JNLP2Viewer.java	1.37 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet.viewer;

import java.awt.BorderLayout;
import java.awt.Canvas;
import java.awt.Frame;
import java.awt.Insets;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import java.security.*;
import java.util.*;
import java.security.AccessController;
import java.security.PrivilegedAction;
import javax.swing.JFrame;
import javax.swing.text.*;
import javax.swing.text.html.*;
import javax.swing.text.html.parser.*;

import sun.awt.*;
import sun.plugin2.applet.*;
import sun.plugin2.applet.context.*;
import sun.plugin2.applet.viewer.util.*;
import sun.plugin2.util.SystemUtil;
import sun.plugin2.main.client.DisconnectedExecutionContext;

import com.sun.deploy.Environment;
import com.sun.deploy.util.*;

import com.sun.javaws.JnlpxArgs;
import com.sun.javaws.Globals;
import com.sun.javaws.jnl.AppletDesc;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.LaunchDescFactory;
import com.sun.javaws.ui.LaunchErrorDialog;

import com.sun.deploy.util.JVMParameters;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.CacheEntry;
import com.sun.deploy.cache.LocalApplicationProperties;
import com.sun.deploy.si.SingleInstanceManager;
import com.sun.deploy.util.Trace;

public class JNLP2Viewer {

    private static final boolean DEBUG = (SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null);

    // Indication of whether we're relaunching an applet that
    // was dragged out of the browser
    private boolean _isDraggedApplet;

    // file association case for -open and -print handling
    private boolean _isAssociation;

    // applet launched via the applet-desc of a JNLP file
    private boolean _isAppletDescApplet;

    public static void main(String[] args) throws Exception {
	// setup Environment to PLUGIN since this method may be 
	// invoked from java webstart
	Environment.setEnvironmentType(Environment.ENV_PLUGIN);
        new JNLP2Viewer().run(args);
    }

    private void run(String[] args) throws Exception {
        Map/*<String, String>*/ params = new HashMap();
        String jnlpFile = null;
        URL documentbase = null;
        String codebase = null;
	String documentbaseArg = null;
	String associationFilename = null;

	if (args.length > 5 || args.length < 1) {
            System.out.println("Usage: JNLP2Viewer [url to HTML page containing <applet> tag with "+JNLP2Tag.JNLP_HREF+" parameter,]");
            System.out.println("                   [url to a jnlp file direct.]");
            System.out.println("Views the first applet on the specified HTML page.");
            System.exit(1);
        }
	
	int idx = 0;
	do {
	    if ((args[idx].equals("-open")) || (args[idx].equals("-print"))) {
		_isAssociation = true;
		associationFilename = args[++idx];
		jnlpFile = args[++idx];
	    } else if (args[idx].equals("-codebase")) {
		_isAppletDescApplet = true;
		codebase = args[++idx];
	    } else if (args[idx].equals("-documentbase")) {
		_isAppletDescApplet = true;
		documentbaseArg = args[++idx];
	    } else if (args[idx].equals("-draggedApplet")) {
		_isDraggedApplet = true;
	    } else {
		// When invoked from javaws, jnlpFiles passed in are all
		// cached jnlp file. See com/sun/javaws/Launcher.java
		jnlpFile = args[idx];
	    }
	    idx++;
	} while (idx < args.length);

	if (DEBUG) {
	    for (int j = 0; j < args.length; j++) {
		System.out.println("\tJNLP2Viewer args[" + j + "] = " + args[j]);
	    }
	}
	
        /**
         * Parse the Applet tag first,
         * which parameters always comes first, i.e. overwrite the jnlp ones
         */
        do {
	    if (_isAppletDescApplet || _isDraggedApplet || _isAssociation) {
		LaunchDesc ld = null;		
		try {
		    ld = LaunchDescFactory.buildDescriptor(new File(jnlpFile),
							   null, null, null);
		} catch (Exception e) {
		    Trace.ignoredException(e);
		}
		if (ld != null) {
		    // if available, use <information><homepage> as document base
		    // NOTE: this is only valid for a webstart jnlp applet
		    // documentbase will be overriden in draggedApplet case from lap
		    documentbase = ld.getInformation().getHome();
		}		
	    } else {
		// This is one argument case. e.g JNLP2Viewer jnlpFile,
		// which is the very original use of JNLP2Viewer
		// Here jnlpFile can be url to a html, url to a jnlp or a jnlp file
		try {
		    // try to see if it is a URL
		    documentbase = new URL(jnlpFile);
		} catch (Exception e) {
		    // must be a jnlp file, change it to file URL
		    // prepend args[0] with "file:///" if it doesn't starts with "http" or "file"
		    if (!jnlpFile.startsWith("http") && !jnlpFile.startsWith("file")) {
			try {
			    documentbase = new URL("file:///" + jnlpFile); 
			    jnlpFile = documentbase.toString();
			} catch (Exception e2) {
			    e2.printStackTrace();
			}
		    } else {
			e.printStackTrace();
		    }
		}
		
		InputStream stream = documentbase.openStream();
		if (stream == null) {
		    throw new RuntimeException("Error opening URL " + documentbase);
		}
		
		AppletTagParser finder = new AppletTagParser();
		new ParserDelegator().parse(new InputStreamReader(stream), finder, true);
		stream.close();
		
		if (!finder.foundApplet()) {
		    if (DEBUG) {
			System.out.println("No applet found on web page, try as JNLP direct");
		    }
		    break;
		}
		
		params = finder.getParameters();
		
		try {
		    jnlpFile = (String) params.get(JNLP2Tag.JNLP_HREF);
		} catch (Exception e) {
		}
		
		if ( jnlpFile == null ) {
		    System.out.println("No <"+JNLP2Tag.JNLP_HREF+"> parameter given in applet tag, bail out\n");
		    System.exit(1);
		}
		
		try {
		    codebase = (String) params.get("java_codebase");
		    if (codebase == null) {
			codebase = (String) params.get("codebase");
		    }
		} catch (Exception e) { }
	    }
	} while (false);

        // make this JVM's parameter accessible deployment wide
        JVMParameters jvmParams = new JVMParameters();
	// all command line options set with jnlpx.vmargs are trusted
	jvmParams.parseTrustedOptions(JnlpxArgs.getVMArgs());
        jvmParams.parseBootClassPath(JVMParameters.getPlugInDependentJars());

        jvmParams.setDefault(true);
        JVMParameters.setRunningJVMParameters(jvmParams);

	if (DEBUG) {
	    System.out.println("Initializing Applet2Environment");
	}
        try {
            sun.net.ProgressMonitor.setDefault(new sun.plugin.util.ProgressMonitor());
        } catch (Throwable e) {
            // This is expected on JDK 1.4.2; don't display any dialog box
        }
	
	//Applet2Environment.initialize() will set "javaplugin.version" property
        Applet2Environment.initialize(null, true, false, 
                                      new Plugin2ConsoleController(null, null),
                                      new NoopExecutionContext(params, null),
				      null);
	
        // static JNLP environment initialization once in a JRE lifetime
        JNLP2Manager.initializeExecutionEnvironment();

	if (_isAppletDescApplet && (documentbaseArg != null)) {
	    // FIXME: which documentbase we should use
	    // the one passed in from javaws launcher
	    // or <homepage href="...."/> we got ealier?
	    documentbase = new URL(documentbaseArg);
	}

	LocalApplicationProperties lap = null;
	if (_isDraggedApplet) {
	    // obtain the LocalApplicationProperties from cache
	    lap = Cache.getLocalApplicationProperties(jnlpFile);
	  
	    // obtain the codebase and documentbase from the lap
	    if (lap != null) {
		codebase = lap.getCodebase();
		documentbase = new URL(lap.getDocumentBase());
	    }
	}
	
	String cachedJNLPFilePath = null;
	if (_isAppletDescApplet || _isDraggedApplet || _isAssociation) {
	    // obtain the original URL from the CacheEntry based on the .idx file
	    // the original URL is required so that an instance of the LaunchDesc
	    // can be created correctly; an instance of LaunchDesc is being created
	    // within the constructor of JNLP2Manager.
	    File idxFile = new File(jnlpFile + ".idx");
	    
	    cachedJNLPFilePath = jnlpFile; 
	    CacheEntry ce = Cache.getCacheEntryFromFile(idxFile);
	    if (ce != null) {
		jnlpFile = ce.getURL();
	    } else {
		if (DEBUG) {
		    System.err.println("Unable to obtain the CacheEntry from file " + jnlpFile + ".idx");
		}
	    }
	}
	
	// setup params containg the name of the file which has been associated with 
	// the JNLP file for the applet so that the applet code can pick it up
	if (_isAssociation) {	    
	    params.put("_numargs", new String("1"));
	    params.put("_arg0", associationFilename);
	    
	}
	
	// save the main ThreadGroup handle or javawsapplicatinthreadgroup if invoked there
	final ThreadGroup mainThreadGroup = Thread.currentThread().getThreadGroup();
	
        final JNLP2Manager manager = new JNLP2Manager(codebase, documentbase, jnlpFile, false); // not relaunched

	if (_isAppletDescApplet || _isDraggedApplet || _isAssociation) {
	    manager.setCachedJNLPFilePath(cachedJNLPFilePath);
	}

        manager.setAppletExecutionContext(new DisconnectedExecutionContext(params, 
                                                                   documentbase.toExternalForm()));

	final boolean isDraggedApplet;
	if (lap != null) {
	    // recheck the isDraggedApplet flag from lap
	    // this is to handle the relaunching of dragged-out applet or relaunching
	    // from file association so that the frame can be made undecorated appropriately
	    isDraggedApplet = lap.isDraggedApplet();
	} else {
	    isDraggedApplet = _isDraggedApplet;
	}
	final boolean isAssociation = _isAssociation;
	final boolean isAppletDescApplet = _isAppletDescApplet;

	AppContext appContext = manager.getAppletAppContext();
	DeployAWTUtil.invokeLater(appContext, new Runnable() {
	    public void run() {
		try {
		    manager.initialize();
		} catch (Exception e) {
		    e.printStackTrace();
		    System.err.println("Error while initializing manager: "+e+", bail out");

		    //show webstart error dialog and exit
		    Environment.setEnvironmentType(Environment.ENV_JAVAWS);
		    LaunchErrorDialog.show(null, e, true);
		}

		Map/*<String, String>*/ params = manager.getAppletExecutionContext().getAppletParameters();

		boolean undecorated = ((isDraggedApplet) ? true : false);

		final LaunchDesc launchDesc = manager.getLaunchDesc();

		int fw = 512;
		int fh = 512;
		try {
		    fw = Integer.parseInt((String) params.get("width"));
		} catch (Exception e) {
		}
		try {
		    fh = Integer.parseInt((String) params.get("height"));
		} catch (Exception e) {
		}

		if (DEBUG) {
		    System.out.println("Starting applet ("+fw+"x"+fh+") with parameters:");
		}


		if (DEBUG) {
		    for (Iterator iter = params.entrySet().iterator(); iter.hasNext(); ) {
			Map.Entry entry = (Map.Entry) iter.next();
			String key = (String) entry.getKey();
			String val = (String) entry.getValue();
			System.out.println("  " + key + " = " + val);
		    }
		}

		// set the title for the JFrame
		String title = null;
		if (launchDesc != null) {
		    title = launchDesc.getInformation().getTitle();
		}
		if (title == null) {
		    title = "JNLPApplet2Viewer";
		}

		// We unilaterally use a JFrame for better support of
		// shaped and translucent applets
		final JFrame f = new JFrame(title);
		// set the frame to be undecorated for dragged out applet
		if (undecorated) {
		    f.setUndecorated(undecorated);
		}
		manager.setAppletSize(fw, fh);
		if (isDraggedApplet) {
		    DragHelper.getInstance().register(manager,
			new DragListener() {
			public void appletDroppedOntoDesktop(Plugin2Manager manager) {
			}
			public void appletExternalWindowClosed(Plugin2Manager manager) {
			    // Dragged applet must call the applet lifecycle
			    // methods stop() and destroy() when it is closed
			    //
			    // manager.stop() has to be called on a thread other than
			    // applet's EDT because the applet thread need run code 
			    // on EDT in order to call applet.stop().
			    stopAndExit(mainThreadGroup, manager);
			}
			});
		}
		f.addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent e) {
			    // Called when alt+f4
			    // Need call manager.stop() as well
			    stopAndExit(mainThreadGroup, manager);
			}
		    });
       
        f.getContentPane().setLayout(new BorderLayout());
           
		manager.setAppletParentContainer(f);
		Insets insets = f.getInsets();
		f.setSize(fw + insets.left + insets.right,
		    fh + insets.right + insets.top);
		if (isAppletDescApplet) {
		    f.setResizable(false);
		}

		manager.addAppletListener(new Applet2Listener() {
		    public boolean appletSSVValidation(Plugin2Manager hostingManager) {
			if (DEBUG) {
			    System.out.println("JNLP2Viewer.appletSSVValidation");

			    Exception e = new Exception("JNLP2Viewer.appletSSVValidation");
			    e.printStackTrace();
			}

			// No SSV support in JNLP2Viewer
			// Note: if JNLP2Viewer is initialized from java webstart
			// SSV is done in webstart's SecureStaticVersioning
			return false;
		    }

		    public boolean isAppletRelaunchSupported() {
			return false;
		    }

		    public void appletJRERelaunch(Plugin2Manager hostingManager, String javaVersion, String jvmArgs) {
			if (DEBUG) {    
			    System.out.println("JNLP2Viewer.appletJRERelaunch:");
			    System.out.println("\tjava_version   : "+javaVersion);
			    System.out.println("\tjava_arguments : "+jvmArgs);
			    
			    Exception e = new Exception("JNLP2Viewer.appletJRERelaunch: "+javaVersion+" ; "+jvmArgs);
			    e.printStackTrace();
			}
		    }

		    public void appletLoaded(Plugin2Manager hostingManager) {
			return;
		    }

		    public void appletReady(Plugin2Manager hostingManager) {
			if (isDraggedApplet) {
			    DragHelper.getInstance().makeDisconnected(manager, f);
			}
			if (isAssociation) {
			    if (SingleInstanceManager.isServerRunning(
				launchDesc.getCanonicalHome().toString())) {
				SingleInstanceManager.connectToServer( launchDesc.toString() );
			    }
			}
			f.setVisible(true);
		    }

		    public void appletErrorOccurred(Plugin2Manager hostingManager) {
			return;
		    }
		    public void released(Plugin2Manager hostingManager) {
			return;
		    }
		    public String getBestJREVersion(Plugin2Manager hostingManager, String javaVersionStr) {
			// not used in JNLP2Viewer
                        return null;
                    }
		});
		manager.start();
	    }
	});
    }

    // start a thread in ThreadGroup group to stop applet and quit the vm
    private void stopAndExit(final ThreadGroup group, final Plugin2Manager manager) {
	AccessController.doPrivileged(new PrivilegedAction() {
		public Object run() {
		    new Thread(group, new Runnable() {
			    public void run() {
				manager.stop(null, null);
				Trace.flush();
				System.exit(0);
			    }
			}).start();
		    return null;
		}
	    });
    }
}
