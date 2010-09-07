/*
 * @(#)JarFileFactory.java	1.35 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www.protocol.jar;

import java.io.*;
import java.net.*;
import java.util.*;
import java.util.jar.*;
import java.util.zip.ZipFile;
import java.security.Permission;

/* A factory for cached JAR file. This class is used to both retrieve
 * and cache Jar files.
 *
 * @author Benjamin Renaud
 * @since JDK1.2
 */
class JarFileFactory {

    /* the url to file cache */
    private static HashMap fileCache = new HashMap();

    /* the file to url cache */
    private static HashMap urlCache = new HashMap();

    URLConnection getConnection(JarFile jarFile) throws IOException {
	URL u = (URL)urlCache.get(jarFile);
	if (u != null)
	    return u.openConnection();

	return null;
    }

    public JarFile get(URL url) throws IOException {
        return get(url, true);
    }

    JarFile get(URL url, boolean useCaches) throws IOException {

	JarFile result = null;
        JarFile local_result = null;

        if (useCaches) {	
            synchronized (this) {
	        result = getCachedJarFile(url);
	    }
	    if (result == null) {
	        local_result = URLJarFile.getJarFile(url);
	        synchronized (this) {
		    result = getCachedJarFile(url);
		    if (result == null) {
		        fileCache.put(url, local_result);
		        urlCache.put(local_result, url);
		        result = local_result;
		    } else {
		        if (local_result != null) {
		            local_result.close();
		        }
	            }
	        }
	    }
        } else {
            result = URLJarFile.getJarFile(url);
        }  
	if (result == null) 
	    throw new FileNotFoundException(url.toString());

	return result;
    }

    private JarFile getCachedJarFile(URL url) {
	JarFile result = (JarFile)fileCache.get(url);

	/* if the JAR file is cached, the permission will always be there */
	if (result != null) {
	    Permission perm = getPermission(result);
	    if (perm != null) {
		SecurityManager sm = System.getSecurityManager();
		if (sm != null) {
		    try {
			sm.checkPermission(perm);
		    } catch (SecurityException se) {
			// fallback to checkRead/checkConnect for pre 1.2
			// security managers
			if ((perm instanceof java.io.FilePermission) &&
			    perm.getActions().indexOf("read") != -1) {
			    sm.checkRead(perm.getName());
			} else if ((perm instanceof 
			    java.net.SocketPermission) &&
			    perm.getActions().indexOf("connect") != -1) {
			    sm.checkConnect(url.getHost(), url.getPort());
			} else {
			    throw se;
			}
		    }
		}
	    }
	}
	return result;
    }

    private Permission getPermission(JarFile jarFile) {
	try {
	    URLConnection uc = (URLConnection)getConnection(jarFile);
	    if (uc != null)
		return uc.getPermission();
	} catch (IOException ioe) {
	    // gulp
	}

	return null;
    }
}
