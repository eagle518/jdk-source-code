/*
 * @(#)PersistenceServiceImpl.java	1.27 04/03/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
import com.sun.javaws.cache.Cache;
import com.sun.javaws.cache.DiskCacheEntry;
import com.sun.deploy.resources.ResourceManager;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import javax.swing.JOptionPane;
import com.sun.jnlp.JNLPClassLoader;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.util.URLUtil;
import java.util.Vector;
import com.sun.deploy.config.Config;

public final class PersistenceServiceImpl implements PersistenceService {
    private long _globalLimit = -1;  // not used currently
    private long _appLimit = -1;
    private long _size = -1;
    static private PersistenceServiceImpl _sharedInstance = null;
    final private SmartSecurityDialog _securityDialog = 
	 new SmartSecurityDialog();

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
	checkAccess(url);
	
	Long maxLen = null;
	try {
	maxLen = (Long) AccessController.doPrivileged(
          new PrivilegedExceptionAction() {
	    public Object run() throws IOException{
		long [] longArray =  Cache.getMuffinAttributes(url);
		if (longArray == null) return new Long(-1);
		
		
		return new Long(longArray[Cache.MUFFIN_MAXSIZE_INDEX]);
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
		    Long longFriendMuffinsSize = null;
		    try {
		    longFriendMuffinsSize = 
			(Long) AccessController.doPrivileged(
			       new PrivilegedExceptionAction() {
			    public Object run() throws IOException{
				return new Long (Cache.getMuffinSize(
						 friendMuffin));
			    }
			});
		    } catch (PrivilegedActionException e) {
			throw (IOException) e.getException();
		    }
		    friendMuffinsTotalMaxSize += 
			longFriendMuffinsSize.longValue();
		} 
	    } 
	}
	
	long totalRequestedMaxSize = maxsize + friendMuffinsTotalMaxSize;

	if (totalRequestedMaxSize  > _appLimit) {
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
		    File f = Cache.getTempCacheFile(url, null);
		    if (f == null) return new Long(-1);
		    Cache.insertMuffinEntry(url, f, 
				 PersistenceService.CACHED, pass_newmaxsize);
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
    
    public FileContents get(URL url)
        throws MalformedURLException, IOException {
	checkAccess(url);
	File f = Cache.getMuffinFileForURL(url);
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
		    DiskCacheEntry dce = Cache.getMuffinEntry(	
					 Cache.MUFFIN_TYPE, url);
		    if (dce == null) {
			throw new FileNotFoundException(url.toString());
		    }
		    Cache.removeMuffinEntry(dce);
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
		    File f = Cache.getMuffinFileForURL(pathUrl);
		    if (!f.isDirectory()) f = f.getParentFile();
		    File [] files = f.listFiles();
		    Vector v = new Vector();
		    for (int i = 0; i < files.length; i++) {
			if (!Cache.isMainMuffinFile(files[i])) continue;
                     DiskCacheEntry dce =
                            Cache.getMuffinCacheEntryFromFile(files[i]);
                        URL url2 = dce.getLocation();
                        File tempF = new File(url2.getFile());
			v.addElement(tempF.getName());
		    }
		    return (String [])v.toArray(new String[0]);
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
		    return new Integer((int)attributes[Cache.MUFFIN_TAG_INDEX]);
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
                    final String s = ResourceManager.getString(
			"APIImpl.persistence.message", 
			new Long(requested), new Long(currentLimit));
		    boolean ret = _securityDialog.showDialog(s);
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
        LaunchDesc ld = JNLPClassLoader.getInstance().getLaunchDesc();
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
	    "APIImpl.persistence.accessdenied", url.toString()));
    }

}



