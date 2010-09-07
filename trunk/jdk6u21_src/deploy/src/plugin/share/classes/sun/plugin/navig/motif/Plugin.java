/*
 * @(#)Plugin.java	1.124 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * Main body of the Java Plug-in process on Un*x systems.
 *
 * This class is launched in a separate process from Navigator.
 * A small stub plugin runs in the Navigator process and sends
 * us requests down a pipe.
 *
 * @author Graham Hamilton
 */

package sun.plugin.navig.motif;

import java.io.*;
import java.awt.*;
import java.util.*;
import java.net.URL;
import sun.plugin.*;
import sun.plugin.viewer.MNetscapePluginContext;
import sun.plugin.viewer.MNetscapePluginObject;
import sun.plugin.viewer.context.*;
import com.sun.deploy.services.PlatformType;
import sun.plugin.viewer.LifeCycleManager;
import sun.awt.SunToolkit;

public class Plugin extends sun.plugin.JavaRunTime {

    /**
     * If tracing is enabled, print the given message.
     */
    public static void trace(String mess) {
        if (tracing) {
            try {
                System.err.println("Child: " + mess);
                tracefile.write(mess);
		tracefile.newLine();
            } catch (IOException ex) {
                System.err.println("Could not write to trace file");
            }
        }
    }

    /* Return the tracing flag */
    public static boolean getTracingFlag() {
        return tracing;
    }

    public static int getNavigVersion() {
        return navig_version;
    }

    public static void error(String mess) {
        System.err.println("                      PLUGIN ERROR ");
        System.err.println("                      ************ ");
        System.err.println(mess);
        System.err.println("\n");
        trace(mess);
    }

    /**
     * Start up the process.  This is called from our "C" main.
     */
    public static void start(boolean t) {
        tracing = t; 
        // force the loading of awt so we get the right libs in the right order
        java.awt.Color c = java.awt.Color.black;

        loadLibrary();	
	String version=System.getProperty("javaplugin.nodotversion");

        /* 
         * See if ProgressON option is specified
         */
        Progress = System.getProperty("progressON");
        
        if (tracing) {
            String trace_file = "plugin_java"+version+".trace";
            try {            	
            	String userName = System.getProperty("user.name", "unknown");
                trace_file = "/tmp/plugin_java"+version+"_"+userName+".trace";
                tracefile = new BufferedWriter(new FileWriter(trace_file));                 
            } catch (IOException ex) {
                error("Could not create "+trace_file);
            }
        }
        String agent  = getenv("JAVA_PLUGIN_AGENT");
        if (agent.indexOf("Mozilla/3") >= 0) {
            trace("Setting Mozilla version to 3");
            navig_version = 3;
        } else if (agent.indexOf("Mozilla/4") >= 0) {
            trace("Setting Mozilla version to 4");
            navig_version = 4;
        } else {
            trace("Setting Mozilla version to 5");
            navig_version = 5;
        }
        Plugin a = new Plugin();
        a.doit();
    }

    private static String evalString(int instance, String jsexp)
    {
        MNetscapePluginObject pluginObj =(MNetscapePluginObject) panels
            .get(new Integer(instance));

	if (pluginObj != null)
	    return pluginObj.evalString(instance, jsexp);
	else
	    return null;
    }
   
    /*
     * This method is the main loop of the process.
     * We sit here reading and executing commands from our parent plugin.
     */
    private void doit() {
        try {
            trace("Plugin class started");

            // Load the appropriate version of the javaplugin library.
	    
            String javaHome = getenv("JAVA_HOME");
            String userHome = getenv("HOME");

            sun.plugin.JavaRunTime.initEnvironment(javaHome, "", userHome);

            String javapluginVersion = getenv("JAVA_PLUGIN_VERSION");
            if (javapluginVersion != null) {
                System.getProperties().put("javaplugin.version", 
                                           javapluginVersion);
            }
         
	    if (getNavigVersion() >= 5.0f)
	    {
	        sun.plugin.AppletViewer.initEnvironment(PlatformType.NETSCAPE6_UNIX);
		encoding = new String("UTF-8");
	    }
	    else
	    {
	        sun.plugin.AppletViewer.initEnvironment(PlatformType.NETSCAPE4_UNIX);
		encoding = new String("ISO-8859-1");
	    }
	                
            // Initializing AppletViewer will have brought up the
            // Java Console (if it is enabled).  So we can print any
            // messages we created earlier:
            trace("Initialized environment. Printing  messages.\n");
            System.out.print(messages);
            
	    Thread watcher = new Watcher();
	    watcher.start();

            initializeCommunication();

            // Now loop, diligently obeying our parent's commands.
            for (;;) {

                // If our parent has died, give up.
                if (!parentAlive()) {
                    trace("parent is dead. Exiting.");
		    onExit();
                    System.exit(4);
                }
                int code = -1;
                trace("Plugin: Reading next  code...");
                try {
                    code = cmdIn.readInt();
                } catch (EOFException eofx) {
                    // Fix for #4654189
                    trace("Pipe got closed, our work is done. Exiting.");
		    onExit();
                    System.exit(4);
                } catch (IOException ex) {
                    // Cylce back to the top and check the parent status
                    trace("Could not read next command code!");
                    continue;
                }
                if (tracing)  
		    trace("VM Received Command >>>" +
			  protocol_to_str(code));
                if (code == JAVA_PLUGIN_NEW) {
                    // Navigator has created a new plugin instance.
                    // Create a corresponding applet viewer.
                    int id = cmdIn.readInt();
                    int bean = cmdIn.readInt();

		    // Obtain parameters
		    int argc = cmdIn.readInt();
		    String[] k = new String[argc];
		    String[] v = new String[argc];

		    for (int i=0; i < argc; i++)
		    {
			k[i] = readString();
			v[i] = readString();

			if (tracing)
			    trace("   >" + k[i] + "==>" + v[i]);
		    }

		    // Create Plugin Object
                    MNetscapePluginObject panel = MNetscapePluginContext.createPluginObject(bean != 0, k, v, id);

                    panels.put(new Integer(id), panel);
                    trace("Registering panel:" + id);
                    replyOK(); 
                } else if (code == JAVA_PLUGIN_START) {
                    int id = cmdIn.readInt();
                    MNetscapePluginObject pluginObj = getPluginObject(id);
                    if (pluginObj != null) 
                        pluginObj.startPlugin();
                } else if (code == JAVA_PLUGIN_STOP) {
                    int id = cmdIn.readInt();
                    MNetscapePluginObject pluginObj = getPluginObject(id);
                    if (pluginObj != null) 
                        pluginObj.stopPlugin();
                } else if (code == JAVA_PLUGIN_DESTROY) {
                    // Navigator is destroying a plugin instance.
                    // Shut down the corresponding applet panel.
                    int id = cmdIn.readInt();
                    MNetscapePluginObject pluginObj = getPluginObject(id);
                    if (pluginObj != null) {
                        Plugin.trace("Removing panel:" + id);
                        panels.remove(new Integer(id));
                        //This try-catch block added to fix: Bugtraq:438855
                        try {
                            pluginObj.destroyPlugin();
                        }
                        catch (Exception ex) {
                            ex.printStackTrace();
                        }
                    }
                    replyOK();

                } else if (code == JAVA_PLUGIN_WINDOW) {
                    // Navigator is sending X11 window information
                    // about a plugin instance.  This incldues its X11
                    // window ID.  pass this through to the
                    // corresponding MNetscapePluginObject
                    int id = cmdIn.readInt();
                    int winid = cmdIn.readInt();
                    int xembed = cmdIn.readInt();
                    int width = cmdIn.readInt();
                    int height = cmdIn.readInt();
                    int x = cmdIn.readInt();
                    int y = cmdIn.readInt();

                    MNetscapePluginObject pluginObj = getPluginObject(id);

                    trace("Window " + winid + " " + width + "x" + height 
                          + " " + x + "x" + y  + " xembed=" + xembed);
                    if (pluginObj != null) 
                        pluginObj.setWindow(winid, xembed, width, height, x, y);
                    replyOK();
                } else if (code == JAVA_PLUGIN_SHUTDOWN) {
                    // Navigator is about to unload our parent plugin.
                    // This will cause it do develop amnesia.  However
                    // we will stick around as a loving and attentive
                    // child, so that if the plugin gets reloaded, it
                    // can start talking to us again.
                } else if (code == JAVA_PLUGIN_DOCBASE) {
                    // When a plugin is first created, navigator doesn't tell
                    // us the docbase.   Our parental plugin plays some tricks
                    // to figure out the docbase value and sends that to us
                    // later.
                    int id = cmdIn.readInt();
                    String docbase = readString();
                    trace("DOCBASE := " + docbase);

                    // Set the docbase for the given panel.
                    MNetscapePluginObject pluginObj = getPluginObject(id);
                    trace("Setting docbase for " + id + " to " + docbase);
                    if (pluginObj != null) {
                        pluginObj.setDocumentURL(docbase);
                    }
                    // We don't need to send a reply.

                } else if (code == JAVA_PLUGIN_PROXY_MAPPING) {
                    String url = readString();
                    String proxy = readString();
                    trace("Plugin.java: PROXY MAPPING: \"" + 
                          url + "\" => \"" +
                          proxy + "\"");
                    Worker.addProxyMapping(url, proxy);

                    // We don't need to send a reply.
                } else if (code == JAVA_PLUGIN_PRINT) {
		    int id = cmdIn.readInt();
		    int x = cmdIn.readInt()/10;
		    int y = cmdIn.readInt()/10;
		    int width = cmdIn.readInt()/10;
		    int height = cmdIn.readInt()/10;
		    trace("PRINT "+ id + " x=" + x + " y=" + y 
			  + " w=" + width + " h=" + height);

		    MNetscapePluginObject pluginObj = getPluginObject(id);
		    if (pluginObj != null) {
			pluginObj.doPrint(x, y, width, height, printOut);
		    }
		    replyOK();
                } else if (code == JAVA_PLUGIN_COOKIE) {
                    int id = cmdIn.readInt();
                    String cookie = readString();
                    Worker.setCookieString(cookie);
                } else if (code == JAVA_PLUGIN_JAVASCRIPT_REPLY) { 
                    int id = cmdIn.readInt();
                    String retval = readString();
                    MNetscapePluginObject pluginObj = getPluginObject(id);
                    if (pluginObj != null) 
                        pluginObj.setJSReply(retval);
                } else if (code == JAVA_PLUGIN_JAVASCRIPT_END) { 
                    int id = cmdIn.readInt();
                    MNetscapePluginObject pluginObj = getPluginObject(id);
                    if (pluginObj != null) 
                        pluginObj.finishJSReply();
                } else if (code == JAVA_PLUGIN_REQUEST_ABRUPTLY_TERMINATED) {
                    Worker.terminateRequestAbruptly();
                } else if (code == JAVA_PLUGIN_ATTACH_THREAD) {
                    trace("Attach Thread ");
                    attachThread();
                } else if (code == JAVA_PLUGIN_GET_INSTANCE_JAVA_OBJECT) {
                    trace("Getting java object");
		    int ind = cmdIn.readInt();
		    MNetscapePluginObject pluginObj = getPluginObject(ind);
		    int res = 0;
                    if (pluginObj != null) { 
		        res = pluginObj.getNativeJavaObject();
		    }
		    replyOK();
		    if (res == 0)
			trace("Return null Java Object");
                    cmdOut.writeInt(res);
                    cmdOut.flush();
		} else if (code == JAVA_PLUGIN_QUERY_XEMBED) {
		    // First, replyOK() to acknowledge that the SendRequest()
		    // is received successfully.
		    replyOK(); 
		    
		    // Next, check to see if it's XToolkit that we're dealing
		    // with.  Send back to client a return value representing
		    // the result of this check.
		    if (SunToolkit.needsXEmbed())
		        cmdOut.writeInt(JAVA_PLUGIN_XEMBED_TRUE);
		    else
			cmdOut.writeInt(JAVA_PLUGIN_XEMBED_FALSE);
		    cmdOut.flush();
		} else if (code == JAVA_PLUGIN_CONSOLE_SHOW) {
		    trace("Showing Java Console");
		    JavaRunTime.showJavaConsole(true);
                    JavaRunTime.installConsoleTraceListener();
		} else if (code == JAVA_PLUGIN_CONSOLE_HIDE) {
		    trace("Hiding Java Console");
		    JavaRunTime.showJavaConsole(false);
                    JavaRunTime.installConsoleTraceListener();
                } else {
                    // We got an unexpected command code.
                    error("Java process: unexpected request " +
                          Integer.toHexString(code));
		    onExit();
		    System.exit(6);
                }

            }

        } catch (Throwable th) {
            // Something has gone wrong.
            // If our parent is still alive, whine about our ills.
            if (parentAlive()) {
                error("Java process caught exception: " + th);
                th.printStackTrace();
            }
            try {
                FileOutputStream fs = 
                    new FileOutputStream("plugin_stack.trace");
                PrintWriter ps = new PrintWriter(fs);
                trace("Java process caught exception: " + th.toString());
                th.printStackTrace(ps);
                ps.flush();
                ps.close();
            } catch (IOException ex) {
                error("Could not print the stack trace\n");
            }
            // In any event, exit.
            trace("Exiting. Navigator may also be dead.");
	    onExit();
            System.exit(6);
        }
    }

   /**
    * onExit()
    * @since 1.4.1
    */
    protected void onExit() {
	LifeCycleManager.destroyCachedAppletPanels();
    }

    void initializeCommunication() {
        trace("Opening pipes at this end\n");

        // Fork a thread to keep an eye on our parent process.
        cmdIn = newInput("Command Input", CmdFD);
        cmdOut = newOutput("Command Output", CmdFD);
        DataInputStream workIn = newInput("Work Input", WorkFD);
        DataOutputStream workOut = newOutput("Work Output", WorkFD);
        
        FileDescriptor fd = getPipe(PrintFD);
        FileOutputStream fout = new FileOutputStream(fd);
        printOut = new PrintStream(fout);

        // Crank the Motif toolkit machinery to life.  This is
        // because we will need to access toolkit resources from
        // natiuve code before we all any AWT methods.
        java.awt.Toolkit tk = Toolkit.getDefaultToolkit();
        trace("Toolkit = " + tk.getClass().getName());
        
        worker = new Worker(workIn, workOut);
        trace("Initialized worker");

	// Initialize spontaneous pipe for Java to JS call
	OJIPlugin.initialize();
        
	// Send a special value on the out pipe to make sure that
	// that is working properly.
        try {
            cmdOut.write(17);
            cmdOut.flush();
            trace("Wrote initial ack on command pipe");
        } catch (IOException ex) {
            error("Error in writing back to the parent");
        }
	trace("Wrote the initial ack\n");
    }

    public static MNetscapePluginObject getPluginObject(int plugin_number) {
	Integer i = new Integer(plugin_number);
	MNetscapePluginObject mpo =  (MNetscapePluginObject) panels.get(i);
	if (mpo == null) {
	    Plugin.error("Could not find a Viewer for " + plugin_number);
	} else {
	    Plugin.trace("Found a viewer for:" + plugin_number);
	}
	return mpo;
    }

    // Create a data input stream for the raw pipe 'fd'
    static DataInputStream newInput(String msg, int fd) {
        Plugin.trace("Creating input pipe:" + msg + " fd = " + fd);
        FileDescriptor fdin = getPipe(fd);
        FileInputStream fin = new FileInputStream(fdin);
        DataInputStream ds =  new DataInputStream(fin);
        return ds;
    }


    // Create a data input stream for the raw pipe 'fd'
    static DataOutputStream newOutput(String msg, int fd) {
        Plugin.trace("Creating output pipe:" + msg + " fd = " + fd);
        FileDescriptor fdout = getPipe(fd);
        FileOutputStream fout = new FileOutputStream(fdout);
        BufferedOutputStream bout = new BufferedOutputStream(fout);
        DataOutputStream dout =  new DataOutputStream(bout);
        return dout;
    }

   
    private class Watcher extends Thread {
        public void run() {
            Plugin.trace(" Starting watcher\n");
            // We want to exit when our parent browser process exits.
            // 
            // We'd like to do this automagically by catching the EOF on
            // the control pipe.
            //
            // Unfortunately because of the way green threads interact with
            // pipe IO, we can't rely on getting woken out of our IO read
            // when the pipe gets closed.
            //
            // So we have to resort to checking on our parent at periodic
            // intervals.  Sigh.

            for (;;) {
                try {
                    Thread.sleep(30 * 1000);
                } catch (InterruptedException ix) {
                    // The Interrupted state is auto-clearing.
                }

                if (!parentAlive()) {
                    trace(" exiting due to parent death");
		    onExit();
                    System.exit(2);
                }
            }
        }
    }


    /**
     * Read an ASCII string from out input pipe.
     */
    private static String readString() throws IOException {
        int len = cmdIn.readUnsignedShort();
        byte buff[] = new byte[len];
        for (int i = 0; i < len; i++) {
            buff[i] = (byte)cmdIn.readByte();
        }
        String res  = new String(buff, encoding);
        trace("readString:" + res);
        return res;
    }

    /* Read an array of bytes from the command pipe */
    private static byte[] readByteArray() throws IOException {
        int len = cmdIn.readInt(); // Length;
        byte buff[] = new byte[len];
        for(int i = 0; i < len; i++) {
            buff[i] = (byte) cmdIn.readByte();
        }
        return buff;
    }

    /**
     * Send an acknowledgement reply to our parent.
     */
    private static void replyOK() throws IOException {
        trace("Sending OK reply");
        cmdOut.writeInt(JAVA_PLUGIN_OK);
        cmdOut.flush();
    }

    /**
     * Load the appropriate version of our native library.
     */
    private static void loadLibrary() {
	String libname = System.getProperty("javaplugin.lib");

        try {
            System.load(libname);
        } catch (UnsatisfiedLinkError ex) {
            System.err.println("Plugin could not load:" + libname);
            // Note the following error message could be deceptive -
            // if the file is not found, it could mean that it had
            // unresolved symbols
            System.err.println("Path is:" + 
                               System.getProperty("java.library.path"));
            System.err.println(ex.toString());
        } 
    }

    static boolean getTracing() { return tracing; }

    private static String encoding;

    private static Hashtable panels = new Hashtable();
    private static int nextId = 0;
    private static boolean tracing;
    private static PrintStream printOut;
    private static DataOutputStream cmdOut;
    private static DataInputStream cmdIn;
    private static Worker worker;
    private static int navig_version;
    private static BufferedWriter tracefile;

    // Queued messages during initialization:
    private static String messages = "";

    // These should be kept consistent with the defs in plugin_defs.h
    public final static int CmdFD = 11;
    public final static int WorkFD = 12;
    public final static int PrintFD = 13;

    // Pointer to an encapsulated version of the OJI specific stuff
    // in the plugin.
    public static OJIPlugin oji;

    // Codes for messages from plugin to java_vm process.
    // Keep consistent with the values in the protocol.h files
    public final static int JAVA_PLUGIN_NEW             	    = 0xFA0001;
    public final static int JAVA_PLUGIN_DESTROY         	    = 0xFA0002;
    public final static int JAVA_PLUGIN_WINDOW          	    = 0xFA0003;
    public final static int JAVA_PLUGIN_SHUTDOWN        	    = 0xFA0004;
    public final static int JAVA_PLUGIN_DOCBASE         	    = 0xFA0005;
    public final static int JAVA_PLUGIN_PROXY_MAPPING   	    = 0xFA0007;
    public final static int JAVA_PLUGIN_COOKIE          	    = 0xFA0008;
    public final static int JAVA_PLUGIN_JAVASCRIPT_REPLY	    = 0xFA000A;
    public final static int JAVA_PLUGIN_JAVASCRIPT_END  	    = 0xFA000B;
    public final static int JAVA_PLUGIN_START           	    = 0xFA0011;
    public final static int JAVA_PLUGIN_STOP            	    = 0xFA0012;
    public final static int JAVA_PLUGIN_ATTACH_THREAD   	    = 0xFA0013;
    public final static int JAVA_PLUGIN_REQUEST_ABRUPTLY_TERMINATED = 0xFA0014;
    public final static int JAVA_PLUGIN_GET_INSTANCE_JAVA_OBJECT    = 0xFA0015;
    public final static int JAVA_PLUGIN_PRINT                       = 0xFA0016;
    public final static int JAVA_PLUGIN_CONSOLE_SHOW                = 0xFA0019;
    public final static int JAVA_PLUGIN_CONSOLE_HIDE                = 0xFA001A;
    public final static int JAVA_PLUGIN_QUERY_XEMBED                = 0xFA001B;

    // Replies from java_vm to plugin.
    public final static int JAVA_PLUGIN_OK                          = 0xFB0001;
    public final static int JAVA_PLUGIN_XEMBED_TRUE                 = 0xFB0002;
    public final static int JAVA_PLUGIN_XEMBED_FALSE                = 0xFB0003;

    static native FileDescriptor getPipe(int fd);
    static native boolean parentAlive();
    
    
    private static native String getenv(String name);
    private static native void attachThread();

    /* Convert the protocol code to a String for debugging purposes */
    public static String protocol_to_str(int code) {
        switch (code) {
        case JAVA_PLUGIN_NEW: return "JAVA_PLUGIN_NEW";
        case JAVA_PLUGIN_DESTROY: return "JAVA_PLUGIN_DESTROY";
        case JAVA_PLUGIN_WINDOW: return "JAVA_PLUGIN_WINDOW";
        case JAVA_PLUGIN_SHUTDOWN: return "JAVA_PLUGIN_SHUTDOWN" ;
        case JAVA_PLUGIN_DOCBASE: return "JAVA_PLUGIN_DOCBASE" ;
        case JAVA_PLUGIN_PROXY_MAPPING: return "JAVA_PLUGIN_PROXY_MAPPING";
        case JAVA_PLUGIN_COOKIE: return "JAVA_PLUGIN_COOKIE     ";
        case JAVA_PLUGIN_JAVASCRIPT_REPLY: 
            return "JAVA_PLUGIN_JAVASCRIPT_REPLY";
        case JAVA_PLUGIN_JAVASCRIPT_END: return "JAVA_PLUGIN_JAVASCRIPT_END";
        case JAVA_PLUGIN_START: return "JAVA_PLUGIN_START";
        case JAVA_PLUGIN_STOP: return "JAVA_PLUGIN_STOP";
        case JAVA_PLUGIN_ATTACH_THREAD: return "JAVA_PLUGIN_ATTACH_THRE";
        case JAVA_PLUGIN_OK: return "JAVA_PLUGIN_OK";
	case JAVA_PLUGIN_PRINT: return "JAVA_PLUGIN_PRINT";
        case JAVA_PLUGIN_REQUEST_ABRUPTLY_TERMINATED            : 
            return "JAVA_PLUGIN_REQUEST_ABRUPTLY_TERMINATED";
	case JAVA_PLUGIN_CONSOLE_SHOW: return "JAVA_PLUGIN_CONSOLE_SHOW";
	case JAVA_PLUGIN_CONSOLE_HIDE: return "JAVA_PLUGIN_CONSOLE_HIDE";
        default:
            return "Unknown code:" + code;
        }

    }
    
    public static String Progress;

}


