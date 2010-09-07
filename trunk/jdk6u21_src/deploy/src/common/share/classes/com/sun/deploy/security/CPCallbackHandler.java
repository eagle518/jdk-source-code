/*
 * @(#)CPCallbackHandler.java	1.8 10/05/19
 *
 * Copyright (c) 2009, 2010 Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * @version 1.0
 */

package com.sun.deploy.security;

import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Vector;
import java.util.jar.Attributes;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.jar.Manifest;
import java.security.AccessController;
import java.security.cert.Certificate;
import java.security.CodeSource;
import java.security.PermissionCollection;
import java.security.PrivilegedExceptionAction;
import sun.misc.JavaLangAccess;
import sun.misc.JavaUtilJarAccess;
import sun.misc.SharedSecrets;
import com.sun.deploy.cache.DeployCacheJarAccess;
import com.sun.deploy.cache.DeployCacheJarAccessImpl;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.ui.UIFactory;

/*
 * This class hooks into the DeployURLClassPath instances
 * used in the dual Applet and JNLP class loaders used in the Java Plugin
 * and Java Web Start. It encapsulates the analysis of trusted content from
 * JAR files as they are opened and loaded into the class path on demand and
 * controls resource discovery along the class path between the dual loaders
 * by applying trust decisions. Trust decisions are determined by the class loader.
 * Currently, classes and other resources are divided into trusted or non-trusted
 * (sandboxed vs. AllPermission) domains. The dual loaders share a common logical
 * class path search order and class path elements are assigned between them.
 * This preserves the original lazy loading performance characteristics.
 *
 * Any type of class path element is allowed but JAR files are handled specially
 * because their contents are easily searchable and static and JAR files are
 * the normal delivery mechanism for trusted content which is downloaded from
 * arbitrary network sites. JAR files are examined for trusted content and
 * namespace assertions are recorded and later guarded as resources are loaded.
 * Untrusted content, either from JAR files or other class path element types,
 * is not analyzed in advance but is rather detected and checked as it is loaded.
 *
 * JAR files with fully trusted contents which contain an "TRUSTED-LIBRARY"
 * main manifest attribute, or which are otherwise recognized as trusted libraries,
 * will be loaded into the parent loader to provide enhanced class loading isolation
 * from potentially untrusted content in the child loader.
 *
 * JAR files with fully trusted contents which contain an "TRUSTED-ONLY"
 * main manifest attribute will be loaded into the child loader class path
 * only if all previously opened class path elements also contain only trusted contents.
 * Likewise, all subsequent class path elements must also contain only trusted
 * contents. Violations result in SecurityExceptions.
 * 
 * A SecurityException will be thrown if a JAR contains any
 * trusted contents which overlaps with previously loaded untrusted contents or if
 * the JAR contains any untrusted content but has a TRUSTED-LIBRARY or TRUSTED-ONLY
 * main manifest attribute.
 */
public class CPCallbackHandler {
    private List childURLs = Collections.synchronizedList(new ArrayList());
    private HashMap assertJars = new HashMap();
    private DeployURLClassPathCallback pcb, ccb;
    private Map resource2trust = new HashMap();
    private Map package2trust = new HashMap();
    private Map defaultCS = new HashMap();
    private Set trustedCS = new HashSet();
    private Set authenticatedCS = new HashSet();
    private Set unauthenticatedCS = new HashSet();
    static CodeSource untrustedCS = new CodeSource(null, (Certificate[]) null);
    private static DeployCacheJarAccess jarAccess = DeployCacheJarAccessImpl.getJarAccess();
    private CPCallbackClassLoaderIf parent;
    private CPCallbackClassLoaderIf child;

    public CPCallbackHandler(CPCallbackClassLoaderIf parent, CPCallbackClassLoaderIf child) {
	this.parent = parent;
	this.child = child;
        pcb  = new ParentCallback();
	ccb  = new ChildCallback();
    }

    public DeployURLClassPathCallback getParentCallback() {
	return pcb;
    }

    public DeployURLClassPathCallback getChildCallback() {
	return ccb;
    }

    /*
     * This is Action Central HQ.
     *
     * All newly opened class path elements are evaluated,
     * trusted code sources are determined, trust-based namespace
     * assertions are recorded and checked, and class path elements
     * are assigned to either the parent or child callback handler.
     *
     * Only fully trusted JAR files containing a main manifest attribute for
     * TRUSTED-LIBRARY are assigned to the parent callback.
     */
    private class ParentCallback extends DeployURLClassPathCallback {
        private boolean trustedChild, authenticatedChild, untrustedChild, trustedOnly;
        private boolean allowMixedTrust;
        private boolean checkMixedTrust;

        private ParentCallback() {
            if (Config.getMixcodeValue() == Config.MIXCODE_ENABLE) {
        	checkMixedTrust = true;
	    }
	    if (!checkMixedTrust) {
                if (Config.getMixcodeValue() == Config.MIXCODE_HIDE_RUN) {
                    allowMixedTrust = true;
	        }
	    }
        }

        public synchronized DeployURLClassPathCallback.Element openClassPathElement(JarFile jar, URL url) throws IOException {
	    jarAccess.setEagerValidation(jar, true);
	    return strategy(jar, url, jarAccess.getCodeSources(jar, url));
        }

        public synchronized DeployURLClassPathCallback.Element openClassPathElement(URL url) throws IOException {
            return strategy(null, url, new CodeSource[] { new CodeSource(url, (Certificate[]) null) });
        }

	/*
	 * Evaluate the new opened parent loader class path element. JAR elements
	 * are eagerly evaluated while non-JAR elements such as the applet codebase
	 * URL are lazily evaluated to avoid unnecessary warnings and security
	 * exceptions. Trusted JAR elements assert ownership over their namespace
	 * in a manner somewhat similar to automated JAR "sealing" and the package
	 * insertion protections in java.lang.ClassLoader.
	 */
	private DeployURLClassPathCallback.Element strategy(JarFile jar, URL url, CodeSource[] sources) {
	    boolean hasTrustedContent = false;
	    boolean hasOnlyTrustedContent = false;
	    boolean maybeTrustedOnly = trustedOnly;
	    boolean maybeTrustedChild = trustedChild;
	    boolean assertsTrustedLibrary = false;
	    boolean assertsTrustedOnly = false;
	    ParentElement pe = new ParentElement(jar, url);

	    if (jar != null) {
	        assertsTrustedLibrary = hasTrustedLibraryAssertion(jar);
	        assertsTrustedOnly = hasTrustedOnlyAssertion(jar);
	    }
	    if (assertsTrustedOnly) {
		Trace.println(url + " is asserting Trusted-Only", TraceLevel.SECURITY);
		if (authenticatedChild || untrustedChild) {
		    pe.setPendingException("attempted to open Trusted-Only jar " + url + " on sandboxed loader");
		    return pe;
		}
	    }

	    CodeSource[] trustedSources = parent.getTrustedCodeSources(sources);
	    if (trustedSources != null && trustedSources.length > 0) {
	        hasTrustedContent = true;
	        if (trustedSources.length == sources.length) {
		    hasOnlyTrustedContent = true;
	        } else {
		    /*
		     * Find untrusted source with no signature. Temporarily,
		     * for backwards compatibility, we treat signed jars
		     * with unsigned META-INF/ resources as if the jar
		     * only contains signed content for the purpose of allowing
		     * Trusted-Library and Trusted-Only. This allows for use
		     * old pre-1.5 jarsigner which fails to add signatures for
		     * such entries. However, if the resources are signed`
		     * we will correctly verify the signature and we will assert
		     * trust on the resources to protect them from name clashes
		     * from other jars.
		     */
		    boolean nonMetaUnsigned = false;
		    for (int i=sources.length-1; i >= 0; i--) {
			if (sources[i].getCertificates() == null) {
			    CodeSource[] unsigned = new CodeSource[] { sources[i] };
			    Enumeration e = jarAccess.entryNames(jar, unsigned);
			    while (e.hasMoreElements()) {
				String name = (String)e.nextElement();
				if (!name.startsWith("META-INF/")) {
				    nonMetaUnsigned = true;
				    break;
				}
			    }
			    if (nonMetaUnsigned) {
				break;
			    }
			}
		    }
		    hasOnlyTrustedContent = !nonMetaUnsigned;
		}
		mergeTrustedSources(trustedSources);
	    }

	    if (hasOnlyTrustedContent) {
	        if (assertsTrustedLibrary) {
                    /*
                     * Apply trusted library assertions to find any pre-existing
                     * conflicts with child loader class path.
                     */
                    if (authenticatedChild || untrustedChild) {
                        pe.setPendingException(assertTrust(jar, trustedSources));
			return pe;
		    } else {
			/* Lazy assertion */
		        assertJars.put(jar, trustedSources);
		    }
                    return pe;
	        }
	        if (assertsTrustedOnly && !trustedOnly) {
		    Trace.println(url + " is newly asserting Trusted-Only", TraceLevel.SECURITY);
		    maybeTrustedOnly = true;
	        }
	    } else {
	        if (assertsTrustedOnly) {
		    pe.setPendingException("attempted to open sandboxed jar " + url + " as Trusted-Only");
		    return pe;
	        }
		if (assertsTrustedLibrary) {
		    pe.setPendingException("attempted to open sandboxed jar " + url + " as a Trusted-Library");
		    return pe;
	        }
	    }
	    if (hasTrustedContent) {
		if (jar != null) {
	            maybeTrustedChild = true;
		}
	    }
            if (maybeTrustedChild && untrustedChild) {
		String msg = checkAllowed(url, maybeTrustedChild && trustedChild);
		if (msg != null) {
		    pe.setPendingException(msg);
		    return pe;
		}
	    }
	    if (authenticatedChild || untrustedChild) {
		/* assert any pending JARs with trusted content */
		if (!assertJars.isEmpty()) {
		    Iterator itor = assertJars.entrySet().iterator();
		    while (itor.hasNext()) {
			Map.Entry entry = (Map.Entry)itor.next();
	                assertTrust((JarFile)entry.getKey(), (CodeSource [])entry.getValue());
		    }
		    assertJars.clear();
		}
	        if (jar != null && hasTrustedContent) {
		    String msg = assertTrust(jar, trustedSources);
		    if (msg != null) {
			pe.setPendingException(msg);
			return pe;
		    }
	        }
            } else {
	        /* Lazy assertion */
	        if (jar != null && hasTrustedContent) {
		    assertJars.put(jar, trustedSources);
	        }
	    }
            childURLs.add(url);
            trustedOnly = maybeTrustedOnly;
            trustedChild = maybeTrustedChild;
	    pe.defer(true);
	    return pe;
	}

	/*
	 * This is called by the checkResource() method of a child
	 * non-JAR class loader path element. It implements lazy
	 * path element evaluation. JAR elements were already
	 * evaluated by strategy().
	 */
	private synchronized void check(URL url, boolean trusted, boolean authenticated) {
	    boolean maybeTrustedChild = trustedChild;
	    boolean maybeUntrustedChild = untrustedChild;
	    boolean maybeAuthenticatedChild = authenticatedChild;
	    if (!trusted && trustedOnly) {
		throw new SecurityException("Trusted-Only loader attempted to load sandboxed resource from " + url);
	    }
	    if (trusted) {
		maybeTrustedChild = true;
	    } else if (authenticated) {
		maybeAuthenticatedChild = true;
	    } else {
		maybeUntrustedChild = true;
	    }
            if (maybeTrustedChild && maybeUntrustedChild) {
		String msg = checkAllowed(url, maybeTrustedChild && trustedChild);
		if (msg != null) {
		    throw new SecurityException(msg);
		}
	    }
	    if (maybeAuthenticatedChild || maybeUntrustedChild) {
		if (!assertJars.isEmpty()) {
		    Iterator itor = assertJars.entrySet().iterator();
		    while (itor.hasNext()) {
			Map.Entry entry = (Map.Entry)itor.next();
	                assertTrust((JarFile)entry.getKey(), (CodeSource [])entry.getValue());
		    }
		    assertJars.clear();
		}
	    }
            trustedChild = maybeTrustedChild;
            authenticatedChild = maybeAuthenticatedChild;
            untrustedChild = maybeUntrustedChild;
	}

        private String checkAllowed(URL url, boolean wasTrusted) {
	    if (checkMixedTrust) {
	        int result = showMixedTrustDialog();
	        if (result == UIFactory.CANCEL) {
	            allowMixedTrust = true;
	        }
	        checkMixedTrust = false;
	    }
	    if (!allowMixedTrust) {
		if (wasTrusted) {
	            return "trusted loader attempted to load sandboxed resource from " + url;
		} else {
	            return "sandboxed loader attempted to load trusted resource from " + url;
		}
	    }
	    return null;
        }
    }

    private class ParentElement extends DeployURLClassPathCallback.Element {
	String pendingException;
	boolean defer;

	ParentElement(JarFile jar, URL url) {
	    super(jar, url);
	}

        public void checkResource(String name) {
	    if (pendingException != null) {
		throw new SecurityException(pendingException);
	    }
	    if (jar == null) {
		throw new SecurityException("invalid class path element " + url + " on Trusted-Library loader");
	    }
        }

	void setPendingException(String msg) {
	    pendingException = msg;
	}

	void defer(boolean defer) {
	    this.defer = defer;
	}

	public boolean defer() {
	    return defer;
	}

	public String toString() {
	    return "defer: " + defer + ", pending: " + pendingException;
	}
    }

    /*
     * All class loading callbacks are performed here except those for
     * fully trusted TRUSTED-LIBRARY JARs.
     *
     * The primary task is to handle resource lookup callbacks from the
     * child class loader's URLClassPath. The checkResource() method
     * is called when a resource is found in the class path element
     * identified by the URL or JarFile parameter. The resource's
     * CodeSource is determined and the trust-based namespace assertion
     * is checked for consistency or a new assertion is recorded.
     */
    private class ChildCallback extends DeployURLClassPathCallback {
        private ChildCallback() {
        }

        public DeployURLClassPathCallback.Element openClassPathElement(JarFile jar, URL url) throws IOException {
	    ChildElement ce = new ChildElement(jar, url);
	    if (childURLs.contains(url)) {
	        if (jar != null) {
	            jarAccess.setEagerValidation(jar, true);
	        }
                return ce;
	    }
	    ce.skip(true);
	    return ce;
        }

        public DeployURLClassPathCallback.Element openClassPathElement(URL url) throws IOException {
	    return openClassPathElement(null, url);
        }
    }

    private class ChildElement extends DeployURLClassPathCallback.Element {
	boolean skip;
	Boolean trusted;
	Boolean authenticated;
	CodeSource cs;

	ChildElement(JarFile jar, URL url) {
	    super(jar, url);
	    if (jar != null) {
                CodeSource[] sources = jarAccess.getCodeSources(jar, url);
		cs = (sources != null) ? sources[0] : null; // should never be null but...
	    } else {
                cs = getDefaultCodeSource(url);
	    }
	    trusted = isTrusted(cs);
	    if (!trusted.booleanValue() && cs.getCertificates() != null) {
		authenticated = isAuthenticated(cs);
	    } else {
		authenticated = Boolean.FALSE;
	    }
	}

        public void checkResource(String name) {
	    CodeSource entryCS;
	    Boolean entryTrusted;
	    Boolean checkTrusted;
	    Boolean entryAuthenticated;
	    String pkg = null;

            /*
             * This name is a "directory" and not an actual resource.
             * Ignore it for backwards compatibility.
             */
            if (name == null || name.endsWith("/")) {
                return;
            }

	    if (jar != null) {
		/*
		 * Jars almost always have a single CodeSource.
		 */
                entryCS = jarAccess.getCodeSource(jar, url, name);

		/*
		 * For backwards compatibility for jars signed with old
		 * jarsigner command, treat unsigned resources under
		 * META-INF/ as trusted for the purposes of the mixed code
		 * warning dialog and the Trusted- manifest attributes if the
		 * first CodeSource in the jar is trusted (signed CodeSources
		 * are always returned before any trailing unsigned CodeSource).
		 *
		 * We still protect against resource/class name trust conflicts.
		 */
		entryTrusted = (entryCS == cs) ? trusted : isTrusted(entryCS);
		entryAuthenticated = (authenticated.booleanValue()) ? authenticated : isAuthenticated(entryCS);
		if (!entryTrusted.booleanValue() && trusted.booleanValue() && name.startsWith("META-INF/")) {
		    checkTrusted = Boolean.TRUE;
		} else {
		    checkTrusted = entryTrusted;
		}
	    } else {
		entryCS = cs;
		entryTrusted = trusted;
		checkTrusted = entryTrusted;
		entryAuthenticated = authenticated;
	    }
	    ((ParentCallback)pcb).check(url, checkTrusted.booleanValue(), entryAuthenticated.booleanValue());
            if (name.endsWith(".class")) {
	        pkg = name.replace('/', '.');
	        pkg = getPackage(pkg.substring(0, pkg.length() - 6));
	    }
	    if (pkg != null) {
	        if (!CPCallbackHandler.this.checkPackage(pkg, entryCS, entryTrusted)) {
		    String clsName = name.replace('/', '.').substring(0, name.length() - 6);
		    String msg = "class \""+ clsName + "\" does not match trust level of other classes in the same package";
		    throw new SecurityException(msg);
		}
            } else {
       	        if (!CPCallbackHandler.this.checkResource(name, entryCS, entryTrusted)) {
		    String msg = "resource \""+ name + "\" does not match trust level of other resources of the same name";
		    throw new SecurityException(msg);
		}
	    }
        }

	void skip(boolean skip) {
	    this.skip = skip;
	}

	public boolean skip() {
	    return skip;
	}
    }

    private boolean hasTrustedLibraryAssertion(JarFile jar) {
	try {
	    Manifest man = jar.getManifest();
	    if (man != null) {
	        Attributes attr = man.getMainAttributes();
		if (attr != null) {
		    Attributes.Name mixed = new Attributes.Name("Trusted-Library");
		    boolean trusted =  Boolean.valueOf(attr.getValue(mixed)).booleanValue();
		    Attributes.Name mixed2 = new Attributes.Name("X-Trusted-Library");
		    boolean xtrusted =  Boolean.valueOf(attr.getValue(mixed2)).booleanValue();
		    if (xtrusted) {
		        Trace.println("old X-Trusted-Library assertion in JAR", TraceLevel.SECURITY);
		    }

		    /* Need to handle case when deployment cache is disabled JDN XXX */
		    return trusted || xtrusted;
		}
	    }
	} catch (IOException ioe) {
	}
	return false;
    }

    private boolean hasTrustedOnlyAssertion(JarFile jar) {
	try {
	    Manifest man = jar.getManifest();
	    if (man != null) {
		Attributes attr = man.getMainAttributes();
		if (attr != null) {
		    Attributes.Name mixed = new Attributes.Name("Trusted-Only");
		    boolean trusted = Boolean.valueOf(attr.getValue(mixed)).booleanValue();
		    Attributes.Name mixed2 = new Attributes.Name("X-Signed-Only");
		    boolean signed = Boolean.valueOf(attr.getValue(mixed2)).booleanValue();
		    if (signed) {
			Trace.println("old X-Signed-Only assertion in JAR", TraceLevel.SECURITY);
		    }

		    return trusted || signed;
		}
	    }
	} catch (IOException ioe) {
	}
	return false;
    }

    /*
     * Spin through the trusted entries, find code packages and non class resources
     * and assert trust in them. The actual package and resource maps associate
     * names with code sources rather than trust. This is an optimization since
     * we expect subsequent lookups to normally have the same code source instance
     * which makes equals() cheap. If sources differ, we then fallback on looking
     * up and comparing trust levels.
     */
    private synchronized String assertTrust(JarFile jar, CodeSource[] trustedSources) {
	Map packages = new HashMap();
	Map resources = new HashMap();
	String name;
	Enumeration trustedSigner;
	String ret = null;

	for (int j=0; j < trustedSources.length; j++) {
	    trustedSigner = jarAccess.entryNames(jar, new CodeSource[] { trustedSources[j] });
	    while (trustedSigner.hasMoreElements()) {
	        name = (String) trustedSigner.nextElement();
	        if (name.endsWith(".class")) {
		    name = name.replace('/', '.');
		    name = getPackage(name.substring(0, name.length() - 6));
		    packages.put(name, trustedSources[j]);
	        } else if (!name.endsWith("/")) {
		    // Must be a resource
		    resources.put(name, trustedSources[j]);
	        }
            }
	}
	Set p = packages.entrySet();
	Set r = resources.entrySet();
        Map.Entry[] pkgs = (Map.Entry[]) p.toArray(new Map.Entry[p.size()]);
        Map.Entry[] res = (Map.Entry[]) r.toArray(new Map.Entry[r.size()]);

	int i;
        i = setTrust(resource2trust, res);
	if (i != -1) {
            ret = "untrusted resource \""+ ((String) res[i].getKey()) + "\" in class path";
	}
        i = setTrust(package2trust, pkgs);
	if (i != -1) {
            unwindTrust(resource2trust, res);
            ret = "untrusted class package \""+ ((String) pkgs[i].getKey()) + "\" in class path";
	}
	return ret;
    }

    private String getPackage(String name) {
	int i = name.lastIndexOf('.');
	return (i == -1) ? "" : name.substring(0, i);
    }

    private int setTrust(Map map, Map.Entry[] entries) {
	CodeSource cs;
	int i;

        for (i=0; i < entries.length; i++) {
            cs = setTrust(map, (String)entries[i].getKey(), (CodeSource)entries[i].getValue());
	    if (cs != null) {
		CodeSource thatCS = (CodeSource) entries[i].getValue();
                if (!cs.equals(thatCS) && isTrusted(cs) != isTrusted(thatCS)) {
		    break;
                }
                entries[i] = null;
	    }
        }

	if (i == entries.length) {
            return -1;
	}
        unwindTrust(map, entries, i);
	return i;
    }

    private CodeSource setTrust(Map map, String name, CodeSource cs) {
	CodeSource thatCS = (CodeSource) map.get(name);
	if (thatCS == null) {
            // first class in this package gets to define
            // trust for all other classes in this package
            map.put(name, cs);
            return null;
	}
        return thatCS;
    }

    private void unwindTrust(Map map, Map.Entry[] entries, int i) {
	// A trust assertion failed. Unwind.
	if (i == 0) {
	    return;
	}
        for (--i; i >= 0; i--) {
	    if (entries[i] != null) {
                map.remove(entries[i].getKey());
	    }
        }
    }

    private void unwindTrust(Map map, Map.Entry[] entries) {
	unwindTrust(map, entries, entries.length);
    }

    synchronized private boolean checkPackage(String name, CodeSource cs, Boolean csTrusted) {
	CodeSource thatCS;
	thatCS = setTrust(package2trust, name, cs);
	if (thatCS != null) {
            if (!thatCS.equals(cs) && isTrusted(thatCS) != csTrusted) {
		return false;
            }
	}
	return true;
    }

    synchronized private boolean checkResource(String name, CodeSource cs, Boolean csTrusted) {
	CodeSource thatCS;
	thatCS = setTrust(resource2trust, name, cs);
	if (thatCS != null) {
            if (!thatCS.equals(cs) && isTrusted(thatCS) != csTrusted) {
		return false;
            }
	}
	return true;
    }

    synchronized private void mergeTrustedSources(CodeSource[] sources) {
	for (int i=0; i < sources.length; i++) {
	    trustedCS.add(sources[i]);
	}
    }

    synchronized private Boolean isTrusted(CodeSource cs) {
	if (cs == untrustedCS) {
	    return Boolean.FALSE;
	}
	return Boolean.valueOf(trustedCS.contains(cs));
    }

    /*
     * Unlike isTrusted() the authenticated status can be upgraded
     * dynamically as signed jars are loaded and their signing certs
     * are granted privileges.
     */
    synchronized private Boolean isAuthenticated(CodeSource cs) {
	if (cs == untrustedCS) {
	    return Boolean.FALSE;
	}
	boolean authenticated = authenticatedCS.contains(cs);
	if (authenticated) {
	    return Boolean.TRUE;
	} else if (!unauthenticatedCS.contains(cs)) {
	    Iterator itor = trustedCS.iterator();
	    while (itor.hasNext()) {
		/*
		 * Try harder...
		 */
		CodeSource source = (CodeSource)itor.next();
		CodeSource target;
		if (Config.isJavaVersionAtLeast15()) {
		    target = new CodeSource(cs.getLocation(), source.getCodeSigners());
		} else {
		    target = new CodeSource(cs.getLocation(), source.getCertificates());
		}
		if (target.equals(cs)) {
		    authenticatedCS.add(cs);
		    return Boolean.TRUE;
		} else {
		    unauthenticatedCS.add(cs);
		}
	    }
	}
	return Boolean.FALSE;
    }

    synchronized private CodeSource getDefaultCodeSource(URL url) {
	if (!trustedCS.isEmpty()) {
	    CodeSource cs;
            cs = (CodeSource) defaultCS.get(url);
	    if (cs == null) {
		cs = new CodeSource(url, (Certificate[]) null);
		defaultCS.put(url, cs);
	    }
	    return cs;
	}
	return untrustedCS;
    }

    private static int showMixedTrustDialog()
    {
        return UIFactory.showMixedCodeDialog(null,null,
                        ResourceManager.getString("security.dialog.mixcode.title"),
                        ResourceManager.getString("security.dialog.mixcode.header"),
                        ResourceManager.getString("security.dialog.mixcode.question"),
                        ResourceManager.getString("security.dialog.mixcode.alert"),
                        ResourceManager.getString("security.dialog.mixcode.buttonYes"),
                        ResourceManager.getString("security.dialog.mixcode.buttonNo"),
                        true);
    }
}


