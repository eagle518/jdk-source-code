/*
 * @(#)DeployCacheJarAccessImpl.java	1.2 10/03/24
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.cache;

import java.io.IOException;
import java.net.URL;
import java.util.Enumeration;
import java.util.jar.JarFile;
import java.security.CodeSource;
import java.security.cert.Certificate;
import sun.misc.JavaUtilJarAccess;
import sun.misc.SharedSecrets;

public class DeployCacheJarAccessImpl implements DeployCacheJarAccess {
    private JavaUtilJarAccess access = SharedSecrets.javaUtilJarAccess();

    static private boolean enhancedJarAccess;

    static {
	try {
	    // Test the waters....
	    JavaUtilJarAccess access = SharedSecrets.javaUtilJarAccess();
	    access.setEagerValidation((JarFile)null, false);
	    enhancedJarAccess = true;
	} catch (NoClassDefFoundError ncdfe) {
	} catch (NoSuchMethodError nsme) {
	} catch (NullPointerException npe) {
	    enhancedJarAccess = true;
	} catch (Exception e) {
	} catch (Error err) {
	}
    }
    public static DeployCacheJarAccess getJarAccess() {
	if (enhancedJarAccess) {
	    return new DeployCacheJarAccessImpl();
	}
	return null;
    }

    public Enumeration entryNames(JarFile jar, CodeSource[] cs) {
	if (jar instanceof CachedJarFile)
	    return ((CachedJarFile)jar).entryNames(cs);
	else if (jar instanceof CachedJarFile14)
            return ((CachedJarFile14)jar).entryNames(cs);
	else
	    return access.entryNames(jar, cs);
    }

    public CodeSource[] getCodeSources(JarFile jar, URL url) {
	if (jar instanceof CachedJarFile)
            return ((CachedJarFile)jar).getCodeSources(url);
	else if (jar instanceof CachedJarFile14)
            return ((CachedJarFile14)jar).getCodeSources(url);
	else
            return access.getCodeSources(jar, url);
    }

    public CodeSource getCodeSource(JarFile jar, URL url, String name) {
	if (jar instanceof CachedJarFile)
            return ((CachedJarFile)jar).getCodeSource(url, name);
	else if (jar instanceof CachedJarFile14)
            return ((CachedJarFile14)jar).getCodeSource(url, name);
	else
            return access.getCodeSource(jar, url, name);
    }

    public void setEagerValidation(JarFile jar, boolean eager) {
	/*
	 * CachedJarFiles are always eagerly validated.
	 */
	if (!(jar instanceof CachedJarFile || jar instanceof CachedJarFile14))
            access.setEagerValidation(jar, eager);
    }
}

