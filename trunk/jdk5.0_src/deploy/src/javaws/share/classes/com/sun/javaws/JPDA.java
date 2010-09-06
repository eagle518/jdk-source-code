/*
 * @(#)JPDA.java	1.11 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.javaws;

import java.net.*;
import java.util.*;
import java.io.IOException;
import javax.swing.JOptionPane;
import com.sun.deploy.config.JREInfo;
import com.sun.deploy.util.DialogFactory;

public class JPDA {

    /*
     *      **********  JPDA  ENVIRONMENT:  **********
     */

    // CONSTANTS:

    // Not A Port Number (or port not available):
    private static final int	NAPN				= -1;

    // Debuggee types:
    public static final int	JWS				= 1;
    public static final int	JWSJNL				= 2;
    public static final int	JNL				= 3;
    private static String	JWS_str				= ""+JWS;
    private static String	JWSJNL_str			= ""+JWSJNL;
    private static String	JNL_str				= ""+JNL;

    // Title for debugging mode notification:
    private static String 	dbgNotificationTitle = "JPDA Notification";

    // ENVIRONMENT:

    // Object representation of JPDA debugging environment for
    // current/next JRE in JRE invocation chain:
    private static JPDA		o_envCurrentJRE			= null;
    private static JPDA		o_envNextJRE			= null;

    // String representation of JPDA debugging environment for
    // current JRE in JRE invocation chain:
    private static String	s_envCurrentJRE			= null;

    // Variables spanning JPDA environment.  Dynamic items -- i.e.,
    // those items that may change in going from one JRE to the next
    // in a JRE invocation chain -- are non-static, i.e., represented
    // by instance variables:
    private static int	_debuggeeType			= 0;
    private static boolean	_jpdaConfigIsFromCmdLine	= false;
    private static String	_portsList			= null;
    private static int[]	_portsPool			= null;
    private int			_selectedPort			= NAPN;
    private boolean		_portIsAutoSelected		= false;
    private String		_excludedportsList		= null;
    private int[]		_excludedportsPool		= null;
    private String		_jreProductVersion		= null;
    private int			_jreNestingLevel		= -1;
    private static boolean	_jreUsesDashClassic		= false;
    private String		_javaMainArgsList		= null;
    //private String[]		_javaMainArgs			= null;

    private static boolean	_nextJreRunsInJpdaMode		= false;

    public static int getDebuggeeType() {
	return _debuggeeType;
    }

    public static void setup() {
	// System property "-Djnlpx.jpda.env=..." was generated
	// by "launcher.c":GetJpdaEnvOption():
	s_envCurrentJRE = getProperty("jnlpx.jpda.env");
	o_envCurrentJRE = decodeJpdaEnv(s_envCurrentJRE);

	// System property "-Djpda.notification" was generated
	// by "launcher.c":ShowJpdaNotificationWindow():
	if (getProperty("jpda.notification") != null) {
	    showJpdaNotificationWindow(o_envCurrentJRE);
	    Main.systemExit(0);
	}
    }

    /*
     * Parses a string representation of the JPDA debugging environ-
     * ment and transforms it into an equivalent, type JPDA object re-
     * presentation.  First, the JPDA instance is newly created, then its
     * variable fields spanning the JPDA environment are set, and finally
     * the object is returned.  Both instance variables (i.e., dynamic
     * JPDA environment items) and static variables (i.e., static JPDA
     * environment items) are set.
     */
    public static JPDA decodeJpdaEnv(String s_env) {
    	JPDA		o_env;
    	String[]	entryTokens, ports;
    	StringTokenizer	st;
    	int		i, len;
    	boolean[]	b;

    	// If there is no debugging env or if it's empty, then
    	// return null  (which also implies that all static
    	// fields in JPDA keep their default/init values, e.g.,
    	// _debuggeeType = 0):
    	if (s_env == null || s_env.equals("")) {
	    return null;
	}

	o_env = new JPDA();

	/* Get individual env entries: */
	st = new StringTokenizer(s_env, "&");
	len = st.countTokens();

	// This introduces a little overhead, but will probably
	// speed up the repeated scanning of the "if"-tests in
	// the  "while () {...}"  block below:
	b = new boolean[len];
	for (i = 0; i < len; i++) { b[i] = true; }

	try {
	    while (st.hasMoreTokens()) {
		/* Get tokens comprising current env entry: */
		entryTokens = tokenizeJpdaEnvEntry(st.nextToken(), "=");

		if (b[0] && entryTokens[0].equals("debuggeeType")) {
		    b[0] = false;
		    if (entryTokens[1].equals(JWS_str)) {
			_debuggeeType = JWS;
			continue;
		    }
		    if (entryTokens[1].equals(JWSJNL_str)) {
                        _debuggeeType = JWSJNL;
                        continue;
                    }
                    if (entryTokens[1].equals(JNL_str)) {
                        _debuggeeType = JNL;
                    }	
                    continue;
                }
                if (b[1] && entryTokens[0].equals("jpdaConfigIsFromCmdLine")) {
                    b[1] = false;
                    if (entryTokens[1].equals("1")) {
                        _jpdaConfigIsFromCmdLine = true;
                    }
                    continue;
                }
                if (b[2] && entryTokens[0].equals("portsList")) {
                    b[2] = false;
                    _portsList = entryTokens[1];
                    if (_portsList.equals("NONE")) {
                        continue;
                    }
                    ports = tokenizeJpdaEnvEntry(_portsList, ",");
                    _portsPool = new int[ports.length];
                    for (i = 0; i < ports.length; i++) {
                        _portsPool[i] = string2Int(ports[i]);
                    }
                    continue;
                }
                if (b[3] && entryTokens[0].equals("selectedPort")) {
                    b[3] = false;
                    o_env._selectedPort = string2Int(entryTokens[1]);
                    continue;
                }
                if (b[4] && entryTokens[0].equals("portIsAutoSelected")) {
                    b[4] = false;
                    if (entryTokens[1].equals("1")) {
                        o_env._portIsAutoSelected = true;
                    }
                    continue;
                }
                if (b[5] && entryTokens[0].equals("excludedportsList")) {
                    b[5] = false;
                    o_env._excludedportsList = entryTokens[1];
                    if (o_env._excludedportsList.equals("NONE")) {
                        continue;
                    }
                    ports = tokenizeJpdaEnvEntry(o_env._excludedportsList, ",");
                    o_env._excludedportsPool = new
                    int[ports.length];
                    for (i = 0; i < ports.length; i++) {
                        o_env._excludedportsPool[i] = string2Int(ports[i]);
                    }
                    continue;
                }
                if (b[6] && entryTokens[0].equals("jreProductVersion")) {
                    b[6] = false;
                    o_env._jreProductVersion = entryTokens[1];
                    continue;
                }
                if (b[7] && entryTokens[0].equals("jreNestingLevel")) {
                    b[7] = false;
                    o_env._jreNestingLevel = string2Int(
                    entryTokens[1]);
                    continue;
                }
                if (b[8] && entryTokens[0].equals("jreUsesDashClassic")) {
                    b[8] = false;
                    if (entryTokens[1].equals("1")) {
                        _jreUsesDashClassic = true;
                    }
                    continue;
                }
                if (b[9] && entryTokens[0].equals("javaMainArgsList")) {
                    b[9] = false;
                    o_env._javaMainArgsList = entryTokens[1];
//                  if (o_env._javaMainArgsList.equals("NONE")) {
//                        continue;
//                  }
//                  o_env._javaMainArgs = tokenizeJpdaEnvEntry(
//                  o_env._javaMainArgsList, ",");
                    continue;
                }
            }
        } catch (NoSuchElementException nsee) {
            return null;
        }
        return o_env;
    }

    // Reverse of decodeJpdaEnv(): transforms an object representation
    // of the JPDA debugging environment into an equivalent string re-
    // presentation, which is then returned.
    public static String encodeJpdaEnv(JPDA o_env) {
	if (o_env == null) return "-Djnlpx.jpda.env";	// empty env
	return	"-Djnlpx.jpda.env=" +
		"debuggeeType=" +
			_debuggeeType +
		"&jpdaConfigIsFromCmdLine=" +
			(_jpdaConfigIsFromCmdLine ? "1" : "0") +
		"&portsList=" +
			_portsList +
		"&selectedPort=" +
			o_env._selectedPort +
		"&portIsAutoSelected=" +
			(o_env._portIsAutoSelected ? "1" : "0") +
		"&excludedportsList=" +
			o_env._excludedportsList +
		"&jreProductVersion=" +
			o_env._jreProductVersion +
		"&jreNestingLevel=" +
			o_env._jreNestingLevel +
		"&jreUsesDashClassic=" +
			(_jreUsesDashClassic ? "1" : "0") +
		"&javaMainArgsList=" +
			o_env._javaMainArgsList;
    }

    /*
     * Sets  _nextJreRunsInJpdaMode  and  o_envNextJRE  to a value that
     * depends on the value of debuggeeType.  Which items of the debugging
     * environment need to be updated for the next JRE will depend on
     * debuggeeType as follows:
     *
     *	0  or  JWS :	no updates required because JNL app won't 
     *			run in debugging mode
     *	JNL	   :	some environment items need to be updated
     *			(e.g., JRE version could change) but not
     *			all (e.g., the selected port shouldn't be
     *			changed because JWS isn't using this port,
     *			i.e., it is't running in debugging mode)
     *	JWSJNL	   :	all environment items need to be updated
     *			(as both JWS and JNL are running in de-
     *			bugging mode
     *
     * NOTE: For debuggeeType == 0 and debuggeeType == JWSJNL, a new
     * (next) JRE *may* be launched, e.g., when the version of the
     * currently running JRE is not the same as the version required
     * to run the JNL app.  For debuggeeType == JWS or debuggeeType ==
     * JNL, we always *force* the relaunch of a new (next) JRE -- see
     * Launcher.java:handleApplicationDesc() -- even if for instance
     * the version of the currently running JRE is the same as the
     * version required to run the JNL app; the forced relaunch is
     * required since there is no other way to switch from running in
     * debugging mode to running in non-debugging mode (JWS) or vice
     * versa (JNL).
     */
    private static void setJpdaEnvForNextJRE(
			boolean		portExclusionAllowed,
			boolean		ephemeralPortAllowed,
			String[]	mainArgs,
			JREInfo 	jreInfo) {
	int	i;
	JPDA	curr, next;

	// debuggeeType == 0 (curr == null)  or  debuggeeType == JWS:

	if (_debuggeeType == 0 || _debuggeeType == JWS) {
	    // no environment change needed, as JNL
	    // app won't run in debugging mode:
	    o_envNextJRE = o_envCurrentJRE;
	    _nextJreRunsInJpdaMode = false;
	    return;
	}

	curr = o_envCurrentJRE;
	next = new JPDA();

	// debuggeeType == JNL  or  debuggeeType == JWSJNL:

	next._jreProductVersion = jreInfo.getProduct();
	next._jreNestingLevel = 1 + curr._jreNestingLevel;
	next._javaMainArgsList = curr._javaMainArgsList;
	if (mainArgs.length > 0) {
	    next._javaMainArgsList = mainArgs[0];
	}
	for (i = 1; i < mainArgs.length; i++) {
	    next._javaMainArgsList += "," + mainArgs[i];
	}

	_nextJreRunsInJpdaMode = true;

	// debuggeeType == JNL:

	if (_debuggeeType == JNL) {
	    next._selectedPort = curr._selectedPort;
	    next._portIsAutoSelected = curr._portIsAutoSelected;
	    next._excludedportsList = curr._excludedportsList;
	    next._excludedportsPool = curr._excludedportsPool;
	    o_envNextJRE = next;
	    return;
	}

	// debuggeeType == JWSJNL :

	if (portExclusionAllowed) {
	    if (curr._excludedportsPool == null) {
		next._excludedportsList = "" + curr._selectedPort;
		next._excludedportsPool = new int[]{curr._selectedPort};
	    }
	    else {
		next._excludedportsList = curr._excludedportsList +
					  "," +
					  curr._selectedPort;
		next._excludedportsPool = new int[
					  curr._excludedportsPool.length + 1];
		for (i = 0; i < curr._excludedportsPool.length; i++) {
		    next._excludedportsPool[i] = curr._excludedportsPool[i];
		}
		next._excludedportsPool[i] = curr._selectedPort;
	    }
	}

	// implicitly/automatically (re)sets next._portIsAutoSelected:
	next._selectedPort = next.getAvailableServerPort(
						portExclusionAllowed,
						ephemeralPortAllowed);

	if (next._selectedPort < 0) {
	    next = null;
	    _nextJreRunsInJpdaMode = false;
	}

	o_envNextJRE = next;
    }


    private static String[] tokenizeJpdaEnvEntry( String envEntry,
						  String delimiters) {
	StringTokenizer st = new StringTokenizer(envEntry, delimiters);
	String[] tokens = new String[st.countTokens()];
	int i;

	try {
	    for (i = 0; st.hasMoreTokens(); i++) {
	    	tokens[i] = st.nextToken();
	    }
	}
	catch (NoSuchElementException nsee) {
	    nsee.printStackTrace();
	    return null;
	}
	return tokens;
    }

    public static void showJpdaNotificationWindow(JPDA o_env) {
	if (o_env == null) {
	    DialogFactory.showErrorDialog( "ERROR: No JPDA environment.",
					    dbgNotificationTitle);
	} else {
	    DialogFactory.showInformationDialog(
		"Starting JRE (version " + o_env._jreProductVersion + ") in JPDA debugging mode, trying server socket port " + o_env._selectedPort + " on this host (" + getLocalHostName() + ").\n\n        Main class  =  " + "com.sun.javaws.Main" + "\n        Arguments to main()  =  " + o_env._javaMainArgsList + "\n\nTo start debugging, please connect a JPDA debugging client to this host at indicated port.\n\n\nDiagnostics:\n\n     Debugging directive was obtained from\n     " + (_jpdaConfigIsFromCmdLine ? "command line:" : "\"javaws-jpda.cfg\" configuration file:") + "\n        - JRE " + (_jreUsesDashClassic ? "uses" : "doesn't use") + "  -classic  option.\n        - Port " + (o_env._portIsAutoSelected ? "automatically selected (by OS);\n          unable to find or use user-specified\n          ports list." : " selected from user-specified list:\n          " + _portsList + "."),
		dbgNotificationTitle + 
		" (" + ((o_env._jreNestingLevel < 1) ? "JWS" : "JNL") + ")");
	}
    }

    private static String getProperty(String key) {
        String property = null;
        try {
            property = System.getProperty(key);
        } catch (SecurityException se) {
            se.printStackTrace();
            return property;
        } catch (NullPointerException npe) {
            npe.printStackTrace();
            return property;
        } catch (IllegalArgumentException iae) {
            iae.printStackTrace();
            return property;
        }
        return property;
    }

    private static int string2Int(String s) {
        int i = -1;
        try {
            i = new Integer(s).intValue();
        } catch (NumberFormatException nfe) {
            nfe.printStackTrace();
            return i;
        }
        return i;
    }
    
    private static String getLocalHostName() {
        try {
            return InetAddress.getLocalHost().getHostName();
        } catch (UnknownHostException uhe) {
            return "localhost";
        }
    }

    /*
     *      **********  PORTS  POOL  MANAGEMENT  **********
     */

    /*
     *
     * This section supplements the functionality in "jpda.c" to manage
     * a pool of ports, allowing the selection of a port not already in
     * use.
     *
     * If getAvailableServerPort() > 0, then from Java Web Start's
     * perspective this means that it is running in JPDA debugging
     * mode.
     *
     * First looks in _portsPool for an available (unused) server port;
     * if exclude=true, then only look for ports that are not contained in
     * _excludedportsPool.  If found, it is returned.  If not found in the
     * pool and dynamic=true, then dynamic (i.e., automatic) allocation of an
     * ephemeral port is tried.  Upon success, that allocated port is returned.
     * If up to this point everything has failed, NAPN is returned.
     */
    public int getAvailableServerPort(boolean exclude, boolean dynamic) {
        int i, p;
        ServerSocket ss;
        if (_portsPool == null) {
            return NAPN;
        }
        _portIsAutoSelected = false;
        for (i = 0; i < _portsPool.length; i++) {
            if ((p = _portsPool[i]) == 0  ||
                (exclude && isExcludedPort(p))) {
		continue;
            }
            try {
                new ServerSocket(p).close();
                return p;
            } catch (IOException ioe) {}
        }
        // ephemeral port selection now our last hope:
        if (dynamic) {
            i = 0;
            try {
                do {
                    ss = new ServerSocket(0);
                    p = ss.getLocalPort();
                    ss.close();
                } while (exclude && isExcludedPort(p));
                _portIsAutoSelected = true;
                return p;
            }
            catch (IOException ioe) {}
        }
        return NAPN;
    }

    private boolean isExcludedPort(int port) {
        int i;
        if (_excludedportsPool == null) return false;
        for (i = 0; i < _excludedportsPool.length; i++) {
            if (port == _excludedportsPool[i]) {
                return true;
            }
        }
        return false;
    }

    /*
     *      **********  LAUNCHING  JNLP  APPS:  **********
     */

    /*
     * This method handles the logic of constructing any relevant debugging
     * arguments to the next to be invoked JRE in the JRE invocation
     * chain.  That invocation will occur in method execProgram(), by
     * calling Runtime.exec() with as its argument the String[] object
     * here referenced by jreExecArgs.  jreExecArgs[0] represents the
     * JRE executable; it is set in execProgram().  The next few members
     * (jreExecArgs[1,2,...]) represent the debugging arguments (if any)
     * for this JRE; they are set up here.
     * The remaining members in jreExecArgs are the remaining original
     * arguments for the JRE, passed in from execProgram.
     *
     * Before setting up the debugging args, setEnvForNextJRE() is called.
     * to ensure that _nextJreRunsInJpdaMode and o_envNextJRE have been 
     * assigned correcty.
     *
     * After setting up the args, showJpdaNotificationWindow is called.
     */
    static public String [] JpdaSetup(String[] args, JREInfo jreInfo) {

	setJpdaEnvForNextJRE(true, true, args, jreInfo);

	if (_nextJreRunsInJpdaMode) {
	    // create a new args array, 
	    // classic has: -classic, -Xnoagent, -Djava.compiler=NONE,
	    // all have: -Xdebug, -Xrunjdwp:transport...
	    int newLength = args.length + ((_jreUsesDashClassic) ? 5 : 2);
	    String[] jreExecArgs = new String[newLength];
	    
	    int n=0;
	    jreExecArgs[n++] = args[0];
	    if (_jreUsesDashClassic) {
		// Note: The "-classic" switch must appear as the
		// first JRE argument (n = 1), otherwise the JRE
		// executable may issue error message  "Unrecognized
		// option: -classic. Could not create the Java virtual
		// machine"  and exit:
		jreExecArgs[n++] = "-classic";
		jreExecArgs[n++] = "-Xnoagent";
		jreExecArgs[n++] = "-Djava.compiler=NONE";
	    }
	    jreExecArgs[n++] = "-Xdebug";
	    jreExecArgs[n++] = "-Xrunjdwp:transport=dt_socket" +
				",server=y,address=" +
				o_envNextJRE._selectedPort +
				",suspend=y";

	    // add any additional args in original arg list (after java cmd)
	    for (int i=1; i<args.length; jreExecArgs[n++]=args[i++]);

	    showJpdaNotificationWindow(o_envNextJRE);
	    return jreExecArgs;
	}
	return args;
    }
}
