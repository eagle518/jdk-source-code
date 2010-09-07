/*
 * @(#)PersistenceServiceImpl.java	1.35 08/01/31
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.net.URL;
import java.net.MalformedURLException;
import javax.jnlp.PersistenceService;
import javax.jnlp.FileContents;
import java.io.File;
import com.sun.deploy.resources.ResourceManager;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import com.sun.jnlp.JNLPClassLoader;
import com.sun.javaws.jnl.LaunchDesc;
import java.util.Vector;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.URLUtil;
import com.sun.deploy.cache.Cache;

public final class PersistenceServiceImpl implements PersistenceService {
    
    final static int MUFFIN_TAG_INDEX = 0;
    final static int MUFFIN_MAXSIZE_INDEX = 1;
    
    private long _globalLimit = -1;  // not used currently
    private long _appLimit = -1;
    private long _size = -1;
    static private PersistenceServiceImpl _sharedInstance = null;

    final private ApiDialog _apiDialog = new ApiDialog();

    private PersistenceServiceImpl() { }
    
    public static synchronized PersistenceServiceImpl getInstance() {
	initialize();
	return _sharedInstance;
    }
    
    public static synchronized void initialize() {
	if (_sharedInstance == null) {
	    _sharedInstance = new PersistenceServiceImpl();
	}
	if (_sharedInstance != null) {
            // global limit not implemented yet
            // _sharedInstance._globalLimit = (long) Config.getIntProperty(
            //     Config.JAVAWS_GLOBAL_MUFFIN_LIMIT_KEY) * (long) 1024;
            _sharedInstance._appLimit = (long) Config.getIntProperty(
                Config.JAVAWS_MUFFIN_LIMIT_KEY) * (long) 1024; 
        }
    }
    
    long getLength(URL url) throws MalformedURLException, IOException {
	checkAccess(url);
	return Cache.getMuffinSize(url);
    }
    
    long getMaxLength(final URL url) throws MalformedURLException, IOException {
	
	Long maxLen = null;
	try {
	maxLen = (Long) AccessController.doPrivileged(
          new PrivilegedExceptionAction() {
	    public Object run() throws IOException{
		long [] longArray =  Cache.getMuffinAttributes(url);
		if (longArray == null) return new Long(-1);
		
		
		return new Long(longArray[MUFFIN_MAXSIZE_INDEX]);
	    }
	});
	} catch (PrivilegedActionException e){
	    throw (IOException) e.getException();
	}
	return maxLen.longValue();
    }
    
    long setMaxLength(final URL url, long maxsize) 
	throws MalformedURLException, IOException {
	// check to make sure maxsize of this entry does not exceed max app size
	long newmaxsize = 0;
	checkAccess(url);
	
	// SBFIX: Hmmm,  checkSetMaxSize might be slow, not sure yet
	
	
	if ((newmaxsize = checkSetMaxSize(url, maxsize)) < 0) return -1;

	final long f_newmaxsize = newmaxsize;
	try {
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
		public Object run() throws MalformedURLException, IOException {
		   Cache.putMuffinAttributes(url, getTag(url),
                           f_newmaxsize);
		    return null;
		}
	    });
	} catch (PrivilegedActionException e) {
	    Exception ee = e.getException();
	    if (ee instanceof IOException) {
		throw (IOException)ee;
	    } else if (ee instanceof MalformedURLException) {
		throw (MalformedURLException)ee;
	    }
	} 
	// SBFIX : IF CURRENT MUFFIN SIZE > newmaxsize WE NEED TO TRUNCATE THE MUFFIN
	return newmaxsize;
    }
    
    private long checkSetMaxSize(final URL url, final long maxsize) 
	throws IOException {
	// algorithm:
	// friendTotalMaxSize = sum of maxsize of all other muffins accessible 
	// by the host that made this muffin.  (i.e. muffins with same 
	// host as this one) 
	// if friendTotalMaxSize + maxsize > application maxsize, ask user if 
	// they want to increase application maxsize.
	URL [] friendMuffins = null;
	try {
	friendMuffins = (URL []) AccessController.doPrivileged(
          new PrivilegedExceptionAction() {
            public Object run() throws IOException{
                
                return (Cache.getAccessibleMuffins(url));
                
            }
	});
	} catch (PrivilegedActionException e) {
	    throw (IOException) e.getException();
	}

	long friendMuffinsTotalMaxSize = 0;
	if (friendMuffins != null) {
	    for (int i = 0; i < friendMuffins.length; i++) {
		if (friendMuffins[i] != null) {
		    final URL friendMuffin = friendMuffins[i];
                    // Don't count the requested muffin itself --
                    // important for correct calculation in setMaxLength().
                    if (! friendMuffin.equals(url)) {
                        long friendMuffinsSize = 0;
                        try {
                            friendMuffinsSize = getMaxLength(friendMuffin);
                        } catch (IOException ex) {
                            // Do nothing - just add 0 for non existing muffin.
                        }
                        friendMuffinsTotalMaxSize += friendMuffinsSize;
                    }
		} 
	    } 
	}
        // Important: don't add maxsize (user parameter) to
        // friendMuffinsTotalMaxSize here, this can lead to arithmetic
        // overflow and security problems.	
        long allowedMaxSize = _appLimit - friendMuffinsTotalMaxSize;

	if (maxsize  > allowedMaxSize) {
	    return reconcileMaxSize(maxsize, friendMuffinsTotalMaxSize, 
				    _appLimit);
	} else {
	    return maxsize;
	}
    }
    
    private long reconcileMaxSize(long maxsize, long friendMuffinsTotalMaxSize,
				  long applimit) {
	long requestedSize = maxsize + friendMuffinsTotalMaxSize;
	
	// add check to see if the askUser() call is needed.
	// no need if the user already grants a signed application 
	// unrestricted access
	final boolean unrestricted = 
		          CheckServicePermission.hasFileAccessPermissions();
	
	if (unrestricted || askUser(requestedSize, applimit)) {
	    _appLimit = requestedSize;
	    return maxsize;
	} else {
	    return applimit - friendMuffinsTotalMaxSize;
	}
    }
    
    private URL [] getAccessibleMuffins(URL url) throws IOException {
	// returns URLs of all muffins that can be accessed by the base 
	// of the passed URL
	return Cache.getAccessibleMuffins(url);
    }
    
    public long create(final URL url, final long maxsize)
	throws MalformedURLException, IOException {
	// compute new limit based on this maxsize + maxsize of all other
	// entries this application has access to.  If this total is
	// >= _limit then ask user if they want to increase the _limit
	checkAccess(url);
	Long l = null;
	
	long newmaxsize = -1;
	if ((newmaxsize = checkSetMaxSize(url, maxsize)) < 0) return -1;
	
	final long pass_newmaxsize = newmaxsize;

	try {    
	    l = (Long)AccessController.doPrivileged(
		new PrivilegedExceptionAction() {
		public Object run() throws MalformedURLException, IOException {		   
                    Cache.createMuffinEntry(url, PersistenceService.CACHED, pass_newmaxsize);
		    return new Long(pass_newmaxsize);
		}
	    });
	} catch (PrivilegedActionException e) {
	    Exception ee = e.getException();
	    if (ee instanceof IOException) {
		throw (IOException)ee;
	    } else if (ee instanceof MalformedURLException) {
		throw (MalformedURLException)ee;
	    }
	}
	
	return l.longValue();
	
    }
    
    public FileContents get(final URL url)
        throws MalformedURLException, IOException {
	checkAccess(url);
        File f = null;
        try {
	    f = (File)AccessController.doPrivileged(
		new PrivilegedExceptionAction() {
		public Object run() throws MalformedURLException, IOException {
		   
		    return Cache.getMuffinFile(url);
		}
            });
	} catch (PrivilegedActionException e) {
	    Exception ee = e.getException();
	    if (ee instanceof IOException) {
		throw (IOException)ee;
	    } else if (ee instanceof MalformedURLException) {
		throw (MalformedURLException)ee;
	    }
	}
	
	if (f == null) throw new FileNotFoundException(url.toString());

	return new FileContentsImpl(f, this, url, getMaxLength(url));
    }
    
    public void delete(final URL url)
	throws MalformedURLException, IOException {
	// delete all entries associated with given URL
	checkAccess(url);
	
	try {
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
		public Object run() throws MalformedURLException, IOException {
	
		    Cache.removeMuffinEntry(url);
		    return null;
		}
	    });
	} catch (PrivilegedActionException e) {
	    Exception ee = e.getException();
	    if (ee instanceof IOException) {
		throw (IOException)ee;
	    } else if (ee instanceof MalformedURLException) {
		throw (MalformedURLException)ee;
	    }
	}

    }
    
    public String[] getNames(URL url) throws MalformedURLException, IOException {
	// return filenames of all entries in a dir
	String [] s = null;
        final URL pathUrl = URLUtil.asPathURL(url);
	checkAccess(pathUrl);
	try {
	    s = (String [])AccessController.doPrivileged(
		new PrivilegedExceptionAction() {
		public Object run() throws MalformedURLException, IOException {
		   
		    return Cache.getMuffinNames(pathUrl);
		}
	    });
	} catch (PrivilegedActionException e) {
	    Exception ee = e.getException();
	    if (ee instanceof IOException) {
		throw (IOException)ee;
	    } else if (ee instanceof MalformedURLException) {
		throw (MalformedURLException)ee;
	    }
	}
	
	return s;
    }
    
    public int getTag(final URL url)
	throws MalformedURLException, IOException {
	// get cached, dirty, or temp for given URL
	Integer i = null;
	checkAccess(url);
	try {
	    i = (Integer)AccessController.doPrivileged(
		new PrivilegedExceptionAction() {
		public Object run() throws MalformedURLException, IOException {
		    long [] attributes = Cache.getMuffinAttributes(url);
		    if (attributes == null) throw new MalformedURLException();
		    return new Integer((int)attributes[MUFFIN_TAG_INDEX]);
		}
	    });
	} catch (PrivilegedActionException e) {
	    Exception ee = e.getException();
	    if (ee instanceof IOException) {
		throw (IOException)ee;
	    } else if (ee instanceof MalformedURLException) {
		throw (MalformedURLException)ee;
	    }
	}

	
	return i.intValue();
    }
    
    public void setTag(final URL url, final int tag)
	throws MalformedURLException, IOException {
	// set cached, dirty, or temp for given URL
	checkAccess(url);
	try {
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
		public Object run() throws MalformedURLException, IOException {
                    
		    Cache.putMuffinAttributes(url,tag,getMaxLength(url));
		    return null;
		}
	    });
	} catch (PrivilegedActionException e) {
	    Exception ee = e.getException();
	    if (ee instanceof IOException) {
		throw (IOException)ee;
	    } else if (ee instanceof MalformedURLException) {
		throw (MalformedURLException)ee;
	    }
	}
	
    }

    /** Actually show the dialogbox */
    private boolean askUser(final long requested, final long currentLimit) {
        Boolean bb = (Boolean)AccessController.doPrivileged(
	    new PrivilegedAction() {
                public Object run() {
		    String title = 
			ResourceManager.getString("api.persistence.title");
                    String msg = 
			ResourceManager.getString("api.persistence.message");

		    String detail = 
			ResourceManager.getString("api.persistence.detail",
			    new Long(requested), new Long(currentLimit));


		    boolean ret = _apiDialog.askUser(title, msg, null,
					null, detail, false);
		    if (ret) {
			long value = Math.min((long)Integer.MAX_VALUE, 
				(requested + 1023)/1024);
			Config.setIntProperty(Config.JAVAWS_MUFFIN_LIMIT_KEY, 
					      (int) value);
			Config.storeIfDirty();
		    }
		    return new Boolean(ret);
                }
            }
	 );
        return bb.booleanValue();
    }




    /** Check accees. The URL must come from the same host as the codebase,
     *  and the access must be a subpath of the codebase
     */  
    private void checkAccess(URL url) throws MalformedURLException {
        // get app codebase
        LaunchDesc ld = JNLPClassLoaderUtil.getInstance().getLaunchDesc();
        if (ld != null) {
            URL codebase = ld.getCodebase();
            if (codebase != null) {
                if (url == null || !codebase.getHost().equals(url.getHost())) {
	            throwAccessDenied(url);
	        }
                String file = url.getFile();
                if (file == null) throwAccessDenied(url); // No name specified
                int idx = file.lastIndexOf('/');
                if (idx == -1) return; // Root access, OK
                if (!codebase.getFile().startsWith(file.substring(0, idx + 1))){
	            throwAccessDenied(url);
	        }
	    }
	}
    }

    private void throwAccessDenied(URL url) throws MalformedURLException {
        throw new MalformedURLException(ResourceManager.getString(
	    "api.persistence.accessdenied", url.toString()));
    }

}



