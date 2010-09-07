/*
 * @(#)CrossDomainXML.java	1.7 10/03/24
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net;

import java.io.BufferedInputStream;
import java.io.InputStream;
import java.io.FileNotFoundException;
import java.net.InetAddress;
import java.net.MalformedURLException;
import java.net.SocketPermission;
import java.net.URL;
import java.net.URLConnection;
import java.security.AccessController;
import java.security.PermissionCollection;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import com.sun.deploy.util.Trace;

/** <P> Implements basic support for cross-domain XML, which provides
    access to web services from unsigned code. See
    http://www.crossdomainxml.org/ for details. </P>

    <P> This implementation only supports the cross-domain policy
    <CODE>&lt;allow-access-from domain="*"&gt;</CODE>. More specific
    access policies are not implemented. This class will return false
    if they are specified. </P>

    <P> Because we do not currently support an analogous API to the
    ActionScript <CODE>System.security.loadPolicyFile</CODE>, we offer
    the ability to load alternate policy files using the
    <CODE>jnlp.altCrossDomainXMLFiles</CODE> system property, which
    takes a comma-separated list of URLs from which policy files may
    be loaded:

<PRE>
 -Djnlp.altCrossDomainXMLFiles=http://photos.googleapis.com/data/crossdomain.xml,http://foo.bar.com/baz/crossdomain.xml
</PRE>

    <P> If the given server is queried and the root crossdomain.xml
    doesn't grant access, the other URLs hosted at the same server
    will be queried to see whether access to the server is granted. We
    do not support any guarding of certain URL sub-trees via this
    mechanism, only general access to the host. </P>
*/

public class CrossDomainXML {
    private CrossDomainXML() {}

    private static final String ALT_XDOMAIN_FILES = "jnlp.altCrossDomainXMLFiles";

    public static final int CHECK_RESOLVE = -1;
    public static final int CHECK_SET_HOST = -2;
    public static final int CHECK_SUBPATH = -3;
    public static final int CHECK_CONNECT = -4;

    // The set of domains we have checked and verified that we are
    // allowed to connect to using sockets
    private static PermissionCollection allowedSocketHosts;

    // The set of domains we have checked and verified that we are
    // allowed to connect to using URLConnection (HTTP)
    private static PermissionCollection allowedURLHosts;

    // Map of the above URLConnection domains to their constrained
    // URL hierarchical subtrees.
    private static final Map/*<String,List>*/ allowedURLs =
	new HashMap/*<String,List>*/();

    // The set of domains for which we have already processed
    // crossdomain files.
    private static final Set/*<String>*/ processedHosts =
	new HashSet/*<String>*/();

    // The set of alternate URLs to query for crossdomain.xml files
    private static List/*<URL>*/ alternateURLs;

    /** Performs a quick check, without making any network connections,
        to see whether access to the given host is allowed. */
    public static synchronized boolean quickCheck(URL url, String host, int port, int mode) {
        // Check for permission to connect raw sockets. Permission to connect
	// a raw socket implies any other mode. Checks for crossdomain.xml
	// permissions for other than raw sockets must go in quickFullCheck()
	// so they are only processed after the plugin/webstart security
	// manager performs the call to super.checkConnect() for raw socket
	// permission.
        if (allowedSocketHosts != null) {
	    if (checkImplies(allowedSocketHosts, host, port, mode)) {
		return true;
	    }
        }

	return false;
    }
        
    /** Performs the full access check, including any necessary
        network connections, to see whether access to the given host
        is allowed. */
    public static boolean check(Class[] context, URL url, final String host, int port, int mode) {
	boolean exception = false;
        try {        
            synchronized (CrossDomainXML.class) {
                if (quickFullCheck(context, url, host, port, mode)) {
                    return true;
                }

                // OK, we need to make a network connection to see whether we
                // can talk to this host

                // Assemble the list of URLs we're going to try
                List urls = new ArrayList();
                boolean changed = false;

                List altURLs = getAlternateURLs();
                if (!processedHosts.contains(host.toLowerCase())) {
                    urls.add(new URL("http", host, "/crossdomain.xml"));
                    processedHosts.add(host.toLowerCase());
                }

                for (Iterator iter = altURLs.iterator(); iter.hasNext();) {
                    URL cur = (URL) iter.next();
                    if (host.equalsIgnoreCase(cur.getHost())) {
                        urls.add(cur);
                        iter.remove();
                    }
                }
            
                // Now process each one
                for (Iterator iter = urls.iterator(); iter.hasNext(); ) {
                    URL cur = (URL) iter.next();
		    if (!check(cur)) {
		        continue;
		    }
                                
                    String socketHost = host;
		    int socketPort = cur.getPort();
		    if (socketPort == -1)
			socketPort = cur.getDefaultPort();
                    if (!socketHost.startsWith("[") && socketHost.indexOf(':') != -1) {
                        socketHost = "[" + socketHost + "]";
                    }

                    if (cur.getPath().equals("/crossdomain.xml")) {
                        // grant raw socket permission for ports > 1023
                        SocketPermission sp = new SocketPermission(socketHost+":1024-", "connect,resolve");
                        if (allowedSocketHosts == null) {
                            allowedSocketHosts = sp.newPermissionCollection();
                        }
                        allowedSocketHosts.add(sp);
                    }

                    // grant URLConnection permission for port of crossdomain.xml
		    List list = (List) allowedURLs.get(host.toLowerCase());
		    if (list == null) {
			list = new ArrayList();
		    }
		    list.add(cur);
                    allowedURLs.put(host.toLowerCase(), list);

                    // grant URL handler socket permission for current URL port
                    SocketPermission sp = new SocketPermission(socketHost+":"+socketPort, "connect,resolve");
                    if (allowedURLHosts == null) {
                        allowedURLHosts = sp.newPermissionCollection();
                    }
                    allowedURLHosts.add(sp);
		    changed = true;
                } // iterator                   

	        // Try again with our awesome new powers
                if (changed && quickFullCheck(context, url, host, port, mode)) {
                    return true;
                }
            } // synchronization
        } catch (Exception e) {
            Trace.ignoredException(e);
        } catch (Throwable t) {
            if (Trace.isEnabled()) {
                t.printStackTrace();
            }   
	    if (t instanceof ThreadDeath)
		throw (ThreadDeath)t;
        }
	return false;
    }
    
    /** Performs a complete check, without making any network connections,
        to see whether access to the given host is allowed. */
    private static boolean quickFullCheck(Class[] context, URL url, String host, int port, int mode) {
        if (quickCheck(url, host, port, mode)) {
	    return true;
	}

	// At this point, full socket access has not been granted so we
	// perform checks which only matter when URLConnection requests
	// are occuring.


	// This only succeeds when full socket permission is granted so
	// stop here.
	if (mode == CHECK_SET_HOST) {
	    return false;
	}

	// A URLConnection request is being made which is granted only due
	// to a crossdomain.xml file. Make sure that any document hierarchy
	// containment is properly enforced.
	if (mode == CHECK_SUBPATH) {
	    if (checkSubpath(url, host, port)) {
		return true;
	    }
	}

	// A URLConnection request is being made which is granted only due
	// to a crossdomain.xml file. Allow the request only if it originates
	// from under the http URLConnection handler.
	if (mode == CHECK_RESOLVE || mode == CHECK_CONNECT) {
	    if (checkContext(context, 
                    sun.net.www.protocol.http.HttpURLConnection.class)) {
                if (allowedURLHosts != null) {
		    if (checkImplies(allowedURLHosts, host, port, mode)) {
		        return true;
		    }
	        }
		return false;
	    }
	}
    
        return false;
    }
                    
    private static boolean checkImplies(PermissionCollection pc, String host, int port, int mode) {
        SocketPermission sp;
	// Handle IPv6 syntax
        if (!host.startsWith("[") && host.indexOf(':') != -1) {
            host = "[" + host + "]";
        }
	if (mode == CHECK_RESOLVE) {
            sp = new SocketPermission(host, "resolve");
	} else  {
            sp = new SocketPermission(host + ":" + port, "connect");
	}
        if (pc.implies(sp)) {
            return true;
        }

	return false;
    }

    private static boolean checkSubpath(URL url, String host, int port) {
        List urls = (List) allowedURLs.get(host.toLowerCase());
	if (urls == null) {
	    return false;
	}

	String path = url.getPath();
	if (path == "")
	    path = "/";
        for (Iterator iter = urls.iterator(); iter.hasNext(); ) {
	    URL cur = (URL) iter.next();
            int allowedPort = cur.getPort();
            if (allowedPort == -1) {
                allowedPort = cur.getDefaultPort();
            }
            // allow http only
	    if (port == allowedPort) {
	        // url must be a subpath of current granted url
		String parentPath = cur.getPath();;
		if (parentPath == "")
		    parentPath = "/";
		int index = parentPath.lastIndexOf('/');
		String prefix;
		if (index != -1) {
		    prefix = parentPath.substring(0, index + 1);
		    if (path.startsWith(prefix)) {
			return true;
		    }
		}
            }
	}

	return false;
    }

    private static boolean checkContext(Class[] context, Class ctxClass) {
        for (int i=0; i < context.length; i++) {
            if (context[i].getClassLoader() != null) {
                return false;
	    }
            if (ctxClass.isAssignableFrom(context[i])) {
                return true;
	    }
        }
        return false;
    }

    private static List getAlternateURLs() {
        // Set up the alternate URLs if necessary
        if (alternateURLs == null) {
            alternateURLs = new ArrayList();
            try {
                String urlList = (String)
                AccessController.doPrivileged(new PrivilegedAction() {
                            public Object run() {
                                return System.getProperty(ALT_XDOMAIN_FILES);
                            }
                        });
                if (urlList != null) {
                    String[] urls = urlList.split(",");
                    for (int i = 0; i < urls.length; i++) {
                        String tmpUrl = urls[i];
                        if (tmpUrl != null) {
                            try {
                                URL tmp = new URL(tmpUrl);
                                // Only obey requests for well known protocols
                                if ("http".equalsIgnoreCase(tmp.getProtocol()) ||
                                    "https".equalsIgnoreCase(tmp.getProtocol())) {
                                         alternateURLs.add(tmp);
                                }
                            } catch (MalformedURLException e) {
                                // Ignore MalformedURLException
                            }
                        }
                    }
                }
            } catch (Throwable t) {
                // Ignore any errors that occur during this parsing
		if (t instanceof ThreadDeath)
		    throw (ThreadDeath)t;
            }
	}

	return alternateURLs;
    }
    private static boolean check(final URL url) {
        try {
            final Handler handler = new Handler();
            AccessController.doPrivileged(new PrivilegedExceptionAction() {
                    public Object run() throws Exception {
                        try {
                            URLConnection conn = url.openConnection();
                            conn.connect();
                            InputStream in = new BufferedInputStream(
                                                 conn.getInputStream());
                            // It's unclear whether it's a good idea to reuse 
                            // the same SAXParserFactory
                            // Use JRE SAX Parser  
                            SAXParserFactory factory = 
                                SAXParserFactory.newInstance("com.sun.org.apache.xerces.internal.jaxp.SAXParserFactoryImpl",
							     null);
                            SAXParser parser = factory.newSAXParser();
                            parser.parse(in, handler);
                            in.close();
                        } catch (FileNotFoundException e) {
			    // no crossdomain.xml on this server
                        } catch (NoSuchMethodError e2) {
			    // SAXParserFactory.newInstance(factoryClassName, classLoader)
			    // is only available since 1.6
			    if (Trace.isEnabled()) {
				Trace.msgNetPrintln("CrossDomainXML: cannot parse crossdomain.xml. You may be running in a JRE older than version 6.0");
			    }
			}
                        return null;
                    }
                });
            return handler.isAllowed();
        } catch (Exception e) {
            Trace.ignoredException(e);
        } catch (Throwable t) {
            if (Trace.isEnabled()) {
                t.printStackTrace();
            }
        }
        return false;
    }

    private static class Handler extends DefaultHandler {
        // States of the parser
        private static final int INITIAL = 0;
        private static final int IN_CROSS_DOMAIN_POLICY = 1;

        // States of the result
        private static final int ALLOWED = 2;
        private static final int DENIED = 3;
        private static final int UNKNOWN = 4;

        private int state = INITIAL;
        private int result = UNKNOWN;

        public void startElement(String uri,
                                 String localName,
                                 String qName,
                                 Attributes attributes) throws SAXException {
            // Parse qName based on current state
            switch (state) {
                case INITIAL: {
                    if (qName.equals("cross-domain-policy")) {
                        state = IN_CROSS_DOMAIN_POLICY;
                    }
                    break;
                }

                case IN_CROSS_DOMAIN_POLICY: {
                    if (qName.equals("allow-access-from")) {
                        // Check to see whether access has been granted to all domains
                        String grantedDomains = attributes.getValue("domain");
                        if (grantedDomains.equals("*") && (result == UNKNOWN)) {
                            result = ALLOWED;
                        } else {
                            // We reject all other constructs
                            result = DENIED;
                        }
                    } else {
                        state = INITIAL;
                    }
                    break;
                }

                default:
                    break;
            }
        }

        public boolean isAllowed() {
            return (result == ALLOWED);
        }
    }

    // Test harness
    public static void main(String[] args) {
        for (int i = 0; i < args.length; i++) {
            System.out.println(args[i] + ": " +
                               (check(new Class[0], null, args[i], -1, CHECK_RESOLVE) ? "Allowed" : "Denied"));
        }
    }
}
