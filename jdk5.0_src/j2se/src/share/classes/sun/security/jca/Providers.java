/*
 * @(#)Providers.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.jca;

import java.util.*;

import java.security.Provider;
import java.security.Security;

/**
 * Collection of methods to get and set provider list. Also includes
 * special code for the provider list during JAR verification.
 *
 * @author  Andreas Sterbenz
 * @version 1.4, 12/19/03
 * @since   1.5
 */
public class Providers {
    
    // InheritableThreadLocal not yet generified, use cast
    private static final ThreadLocal<ProviderList> threadLists =
    	(ThreadLocal<ProviderList>)((ThreadLocal)new InheritableThreadLocal());
	
    // number of threads currently using thread-local provider lists
    // tracked to allow an optimization if == 0
    private static volatile int threadListsUsed;
    
    // current system-wide provider list
    // Note volatile immutable object, so no synchronization needed.
    private static volatile ProviderList providerList;
    
    static {
	// set providerList to empty list first in case initialization somehow
	// triggers a getInstance() call (although that should not happen)
	providerList = ProviderList.EMPTY;
	providerList = ProviderList.fromSecurityProperties();
    }
    
    private Providers() {
	// empty
    }

    // we need special handling to resolve circularities when loading
    // signed JAR files during startup. The code below is part of that.
    
    // Basically, before we load data from a signed JAR file, we parse
    // the PKCS#7 file and verify the signature. We need a
    // CertificateFactory, Signatures, etc. to do that. We have to make
    // sure that we do not try to load the implementation from the JAR
    // file we are just verifying.
    //
    // To avoid that, we use different provider settings during JAR
    // verification. Those are setup to include all providers already
    // instantiated plus the Sun and SunRsaSign providers for initial
    // bootstrapping.
    //
    // However, we do not want those provider settings to interfere
    // with other parts of the system. Therefore, we make them local
    // to the Thread executing the JAR verification code.
    //
    // The code here is used by sun.security.util.SignatureFileVerifier.
    // See there for details.
    
    // hardcoded classnames of providers to use for JAR verification
    // MUST NOT be in signed JAR files
    private static final String[] jarClassNames = {
	"sun.security.provider.Sun",
	"sun.security.rsa.SunRsaSign",
    };
    
    // flag object
    private final static Object ALL_PROVIDERS_LOADED = new Object();
    
    /**
     * Start JAR verification. This sets a special provider list for
     * the current thread. You MUST save the return value from this
     * method and you MUST call stopJarVerification() with that object
     * once you are done.
     */
    public static Object startJarVerification() {
	ProviderList currentList = getProviderList();
	ProviderList jarList = currentList.getJarList(jarClassNames);
	if (jarList == null) {
	    return ALL_PROVIDERS_LOADED;
	}
	if (ProviderList.debug != null) {
	    ProviderList.debug.println("ThreadLocal providers: " + jarList);
	}
	// return the old thread-local provider list, usually null
	ProviderList threadList = getThreadProviderList();
	threadListsUsed++;
	setThreadProviderList(jarList);
	return threadList;
    }
    
    /**
     * Stop JAR verification. Call once you have completed JAR
     * verification
     */
    public static void stopJarVerification(Object obj) {
	if (obj == ALL_PROVIDERS_LOADED) {
	    return;
	}
	if (ProviderList.debug != null) {
	    ProviderList.debug.println("Disabling ThreadLocal providers");
	}
	// restore old thread-local provider list
	ProviderList list = (ProviderList)obj;
	setThreadProviderList(list);
	threadListsUsed--;
    }
    
    /**
     * Return the current ProviderList. If the thread-local list is set,
     * it is returned. Otherwise, the system wide list is returned.
     */
    public static ProviderList getProviderList() {
	ProviderList list = getThreadProviderList();
	if (list == null) {
	    list = getSystemProviderList();
	}
	return list;
    }
    
    /**
     * Set the current ProviderList. Affects the thread-local list if set,
     * otherwise the system wide list.
     */
    public static void setProviderList(ProviderList newList) {
	if (getThreadProviderList() == null) {
	    setSystemProviderList(newList);
	} else {
	    setThreadProviderList(newList);
	}
    }

    /**
     * Get the full provider list with invalid providers (those that
     * could not be loaded) removed. This is the list we need to
     * present to applications.
     */
    public static synchronized ProviderList getFullProviderList() {
	ProviderList list = getThreadProviderList();
	if (list != null) {
	    ProviderList newList = list.removeInvalid();
	    if (newList != list) {
		setThreadProviderList(newList);
		list = newList;
	    }
	    return list;
	}
	list = getSystemProviderList();
	ProviderList newList = list.removeInvalid();
	if (newList != list) {
	    setSystemProviderList(newList);
	    list = newList;
	}
	return list;
    }
    
    private static ProviderList getSystemProviderList() {
	return providerList;
    }
    
    private static void setSystemProviderList(ProviderList list) {
	providerList = list;
    }
    
    private static ProviderList getThreadProviderList() {
	// avoid accessing the threadlocal if none are currently in use
	// (first use of ThreadLocal.get() for a Thread allocates a Map)
	if (threadListsUsed == 0) {
	    return null;
	}
	return threadLists.get();
    }
    
    private static void setThreadProviderList(ProviderList list) {
	if (list == null) {
	    threadLists.remove();
	} else {
	    threadLists.set(list);
	}
    }
    
}

