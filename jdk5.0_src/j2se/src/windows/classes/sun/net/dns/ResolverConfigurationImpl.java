/*
 * @(#)ResolverConfigurationImpl.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.dns;

import java.util.List;
import java.util.LinkedList;
import java.util.StringTokenizer;
import java.io.IOException;

/*
 * An implementation of sun.net.ResolverConfiguration for Windows.
 */

public class ResolverConfigurationImpl
    extends ResolverConfiguration
{
    // Lock helds whilst loading configuration or checking
    private static Object lock = new Object();

    // Resolver options
    private final Options opts;

    // Addreses have changed
    private static boolean changed = false;

    // Time of last refresh.
    private static long lastRefresh = -1;    

    // Cache timeout (120 seconds) - should be converted into property
    // or configured as preference in the future.
    private static final int TIMEOUT = 120000;

    // DNS suffix list and name servers populated by native method
    private static String os_searchlist;
    private static String os_nameservers;

    // Cached lists
    private static LinkedList searchlist;
    private static LinkedList nameservers;

    // Parse string that consists of token delimited by space or commas
    // and return LinkedHashMap
    private LinkedList stringToList(String str) {
	LinkedList ll = new LinkedList();

	// comma and space are valid delimites
	StringTokenizer st = new StringTokenizer(str, ", ");
	while (st.hasMoreTokens()) {
	    String s = st.nextToken();
	    if (!ll.contains(s)) {
		ll.add(s);
	    }
	}
	return ll;
    }

    // Load DNS configuration from OS

    private void loadConfig() {
	assert Thread.holdsLock(lock);

	// if address have changed then DNS probably changed aswell;
	// otherwise check if cached settings have expired.
	//
	if (changed) {
	    changed = false;
	} else {
	    if (lastRefresh >= 0) {
		long currTime = System.currentTimeMillis();
		if ((currTime - lastRefresh) < TIMEOUT) {
		    return;
		}
	    }
	}

	// load DNS configuration, update timestamp, create
	// new HashMaps from the loaded configuration
	//
	loadDNSconfig0();

	lastRefresh = System.currentTimeMillis();
	searchlist = stringToList(os_searchlist);
	nameservers = stringToList(os_nameservers);
	os_searchlist = null;			    // can be GC'ed
	os_nameservers = null;
    }

    ResolverConfigurationImpl() {
	opts = new OptionsImpl();
    }

    public List searchlist() {
	synchronized (lock) {
	    loadConfig();

	    // List is mutable so return a shallow copy
	    return (List)searchlist.clone();
	}
    }

    public List nameservers() {
	synchronized (lock) {
	    loadConfig();

	    // List is mutable so return a shallow copy
	    return (List)nameservers.clone();
	 }
    }

    public Options options() {
	return opts;
    }

    // --- Address Change Listener 

    static class AddressChangeListener extends Thread {
	public void run() {
	    for (;;) {
		// wait for configuration to change
		if (notifyAddrChange0() != 0)
		    return;	
		synchronized (lock) {
		    changed = true;
		}
	    }
	}
    }


    // --- Native methods --

    static native void init0();
   
    static native void loadDNSconfig0();

    static native int notifyAddrChange0();

    static {
	java.security.AccessController.doPrivileged(
	    new sun.security.action.LoadLibraryAction("net"));
	init0();

	// start the address listener thread
	AddressChangeListener thr = new AddressChangeListener();
	thr.setDaemon(true);	
	thr.start();
    }
}

/**
 * Implementation of {@link ResolverConfiguration.Options}
 */
class OptionsImpl extends ResolverConfiguration.Options {
}
