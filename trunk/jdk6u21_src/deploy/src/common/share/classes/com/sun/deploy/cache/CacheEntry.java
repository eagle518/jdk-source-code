/*
 * @(#)CacheEntry.java	1.80 09/12/09
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.deploy.cache;

import java.io.File;
import java.io.FileOutputStream;
import java.io.RandomAccessFile;
import java.io.IOException;
import java.io.ByteArrayOutputStream;
import java.io.ObjectOutputStream;
import java.io.ObjectInputStream;
import java.io.ObjectStreamClass;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.ByteArrayInputStream;
import java.io.BufferedWriter;
import java.io.OutputStreamWriter;
import java.io.InputStream;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.FileNotFoundException;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.Map;
import java.util.Iterator;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.ArrayList;
import java.util.Map.Entry;
import java.util.Set;
import java.util.jar.JarFile;
import java.util.jar.Attributes;
import java.util.jar.Attributes.Name;
import java.util.jar.Manifest;
import java.util.jar.JarEntry;
import java.util.StringTokenizer;
import java.util.zip.GZIPOutputStream;
import java.util.zip.GZIPInputStream;
import java.security.CodeSigner;
import java.security.CodeSource;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.security.cert.CertPath;
import java.security.cert.CertificateFactory;
import java.security.Timestamp;
import java.security.cert.CertificateException;
import java.security.Principal;
import java.lang.ref.SoftReference;
import java.net.URL;
import java.net.InetAddress;
import com.sun.deploy.config.Config;
import com.sun.deploy.net.HttpRequest;
import com.sun.deploy.util.SyncFileAccess;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.net.JARSigningException;
import com.sun.deploy.net.MessageHeader;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.Environment;
import com.sun.deploy.util.BlackList;
import com.sun.deploy.util.TrustedLibraries;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import sun.misc.JavaUtilJarAccess;
import sun.misc.SharedSecrets;

import java.net.URLClassLoader;

/*
   Cache Index file implementation
   Each cached resource in the cache with have it's own index file
 
   Index File Structure:

   Section 1 (mandatory header, 128 bytes):
     Busy (1 byte)
     Complete (1 byte)
     Cache version (int)
     Force update (1 byte)
     No-href (1 byte)
     Is shortcut image (1 byte)
     Content-Length (long)
     Last modified date (long)
     Expiration date (long)

     Cache entry creation time (long)
     Security validation timestampt (long)
     Cached security validation state (1 byte)

     Length of section 2 (int)
     Length of section 3 (int)
     Length of section 4 (int)
     Length of section 5 (int)

     Blacklist validation timestampt (long)
     Certificate expiration date (long)

     Length of section 4 pre-1.5 cert info (int)
     Contains unsigned entries (byte)
     Single code source (byte)
     Length of section 4 certs only (int)
     Length of section 4 code signers only (int)
     Contains signing info for missing zip entries (byte)

   Section 2 (String header data, mandatory):
     Version ID (UTF-String)
     URL (UTF-String)
     NameSpace ID (UTF-String)
     HTTP header (if header is available)

   Section 3 (Jar only):
     Manifest
 
   Section 4 (Jar only):
     Pre-1.5 Certificate information
     1.5+ CodeSigner information
     (TimeStamp information is included in the CodeSigner object already)

   Section 5 (Additional data needed to cache results of security validation)
             (mostly for JNLP files):
     List of resources and their validation timestamps.
     (if all resources were not changed and still have same timestamps then we
      can reuse previous result of validation)

  NB on changing index file format:
    - adding new fields to unused part of the index header is OK and does not
      require changing cache version number
      (zero bytes will mean "from older cache")
    - other changes in the format require change of cache version number
    - first 6 bytes (busy, incomplete, cache version)
      are reserved for backward compatibility and MUST not be changed
 */
public class CacheEntry {    
    private File indexFile = null;    
    private SyncFileAccess indexFileSyncAccess = null;
    private File tempDataFile = null;    
    private int busy = 1;
    private int incomplete = 0;
    private int forceUpdate = 0;
    private int noHref = 0;
    private int cacheVersion = Cache.getCacheVersion();
    private int contentLength = 0;
    private int isShortcutImage = 0;
    private long lastModified = 0;
    private long expirationDate = 0;
    private String version = null;
    private String url = "";
    private String namespaceID = "";
    private MessageHeader headerFields = new MessageHeader();
    private String filename = null;
    private String codebaseIP = null;

    private long entryCreationTime = 0; /* time when cache entry for resource
                                       was created and filed with data */
    private long validationTimestampt = 0; /* time of last security validation */
    private long certExpirationDate = 0;   /* time till all certificates used 
                                              for security validation are valid 
                                              (safe to use cached values) */
    private boolean knownToBeSigned = false; /* cached result of security verification */
    private long blacklistValidationTime = 0; /* last time checked
                                                 (only for signed jars) */
    private long trustedLibrariesValidationTime = 0; /* last time checked
                                                 (only for signed jars) */

    public static final byte PREVERIFY_FAILED = 2;
    public static final byte PREVERIFY_SUCCEEDED = 1;
    public static final byte PREVERIFY_NOTDONE = 0;
    
    private byte classVerificationStatus = PREVERIFY_NOTDONE;

    Map checkedJars = null;

    private boolean hasOnlySignedEntries = false;
    private boolean hasSingleCodeSource = false;
    private boolean hasMissingSignedEntries = false;

    /* section1 has fixed length */
    private final static int section1Length = 128;
    private int section2Length = 0;
    private int section3Length = 0;
    private int section4Length = 0;
    private int section4CertsLength = 0;
    private int section4SignersLength = 0;
    private int section4Pre15Length = 0;
    private int section5Length = 0;
    private int reducedManifestLength = 0;  // 6u18 only
    private int reducedManifest2Length = 0; // 6u19+
    
    private final static String META_FILE_DIR = "META-INF/";
    
    private final static String JAR_INDEX_NAME = "META-INF/INDEX.LIST";

    /* State specific to JARs
     * 
     * We want to have it around until it is used but not much longer because 
     * it may occupy a lot of RAM. If subsequent request for this entry will 
     * happen after a while we can always recreate these structures from data
     * on the disk. This will require some CPU cycles but this should not 
     * happen often unless we are really low on memory and in such case 
     * we will fail with OutOfMemory otherwise anyway.
     * 
     * Note that all references are null until manifest is read for first time.
     * After that if any of these variables is assigned to null this means 
     * that it is not present and there is no need to try to reload it.
     */
    private SoftReference manifestRef = null;
    private boolean doneReadManifest = false;
    private boolean doneReadCerts = false;
    private boolean doneReadSigners = false;
    
    // for 1.5 plus
    private Map signerMapHardRef = null;
    private SoftReference signerMapRef = null; 

    private CodeSigner[] signersHardRef = null;
    private SoftReference signersRef = null;

    private Map codeSourceCacheHardRef = null;
    private SoftReference codeSourceCacheRef = null;

    
    // for 1.4 or below
    private Map signerMapCertHardRef = null;
    private SoftReference signerMapCertRef; 

    private Certificate[] certificatesHardRef = null;
    private SoftReference certificatesRef = null;

    private Map codeSourceCertCacheHardRef = null;
    private SoftReference codeSourceCertCacheRef = null;

    static private boolean enhancedJarAccess;

    static public boolean hasEnhancedJarAccess() {
	return enhancedJarAccess;
    }

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

    public void verifyJAR(URLClassLoader verifyCL) {
        JarFile jf = getJarFile();
        if (jf == null) {
            return;
        }
        Enumeration entries = jf.entries();
        boolean verificationStarted = false;
        boolean verificationErrorEncountered = false;
        while (entries.hasMoreElements()) {
            JarEntry entry = (JarEntry) entries.nextElement();
            String name = entry.getName();
            if (name != null && name.endsWith(".class")) {

                try {
                    String className = name.substring(0, name.lastIndexOf(".class"));

                    // load the class for the class verification
                    // this do not initialize the class
                    Class c = Class.forName(className.replace('/', '.'),
                            false, verifyCL);

                    verificationStarted = true;
                } catch (Throwable t) {      
                    boolean ignoreError = false;
                    String error = t.getMessage().replace('/', '.');
                    if (error != null &&
                            (error.indexOf("com.sun.media.jmcimpl.JMFPlayerPeer") != -1 ||
                            error.indexOf("javafx.fxunit.FXTestCase") != -1 ||
                            error.indexOf("javax.media.ControllerListener") != -1 ||
                            error.indexOf("junit.framework.TestCase") != -1)) {

                        try {
                            URL u = new URL(getURL());
                            String host = u.getHost();
                            if (host.equals("dl.javafx.com")) {
                                // ignore these classes if they are comming
                                // from dl.javafx.com
                                // they are currently in javafx runtime JAR, but is
                                // never used and they reference to other classes that
                                // is not part of the JavaFX runtime.
                                // Once these classes are removed from the JavaFX runtime
                                // JAR, this should be removed.
                                Trace.println("CacheEntry:  Skipped verification for class " + error + " in " + getURL(),
                                        TraceLevel.CACHE);
                                ignoreError = true;
                            }
                        } catch (Exception e) {
                            // should not happen
                        }
                    }
                    if (Environment.allowAltJavaFxRuntimeURL()) {
                        ignoreError = true;
                        if (Trace.isTraceLevelEnabled(TraceLevel.CACHE)) {
                            t.printStackTrace();
                        }
                    }
                    if (!ignoreError) {
                        // class verification failed
                        Trace.println("Class verification failed: " +
                                t.getMessage() + " for " + getURL(),
                                TraceLevel.CACHE);
                        verificationErrorEncountered = true;
                        break;
                    }
                }
            }
        }

        if (verificationStarted && verificationErrorEncountered == false) {
            updateClassVerificationStatus(PREVERIFY_SUCCEEDED);
            Trace.println("CacheEntry:  Pre-verify done for all classes in " +
                    getURL(), TraceLevel.CACHE);
            return;
        }

        updateClassVerificationStatus(PREVERIFY_FAILED);
        Trace.println("CacheEntry:  Cannot pre-verify all classes in " +
                getURL(), TraceLevel.CACHE);
    }

    public long getCreationTimespampt() {
        return entryCreationTime;
    }
    
    private boolean isOKToUseCachedSecurityValidation() {
        if (System.currentTimeMillis() > certExpirationDate) {
            return false;
        }
        //if revocation is possible then we can not rely on fact 
        // that certificate was ok last time ...
        //For now simply disable caching in this case
        if (Config.getBooleanProperty(Config.SEC_USE_VALIDATION_CRL_KEY) ||
            Config.getBooleanProperty(Config.SEC_USE_VALIDATION_OCSP_KEY)) {
            Trace.println("Certificate revocation enabled. Disable security validation optimizations.", TraceLevel.SECURITY);
            return false;
        }
        return true;
    }

    public long getValidationTimestampt() {
        if (!isOKToUseCachedSecurityValidation()) {
            knownToBeSigned = false;
            validationTimestampt = 0;
        }
        return validationTimestampt;
    }
    
    private void updateBlacklistValidation() {
	if (Config.getBooleanProperty(Config.SEC_USE_BLACKLIST_CHECK_KEY)) {
            blacklistValidationTime = System.currentTimeMillis();
	}
    }

    private void updateTrustedLibrariesValidation() {
        trustedLibrariesValidationTime = System.currentTimeMillis();
    }

    /* returns true if we know that this jar has been properly
       validated and result was true */
    public boolean isKnownToBeSigned() {
        if (!isOKToUseCachedSecurityValidation()) {
            //certificate could be expired. should not use cached values
            knownToBeSigned = false;
            validationTimestampt = 0;
        }
        return knownToBeSigned;
    }

    public byte getClassesVerificationStatus() {
        return classVerificationStatus;
    }

    private void updateClassVerificationStatus(byte status) {
        classVerificationStatus = status;
        try {
            updateIndexHeaderOnDisk();
        } catch (IOException e) {
            Trace.println("Failed to update Class Verification result in the index", TraceLevel.CACHE);
            Trace.ignoredException(e);
        }
    }

    /* returns true if jar has signing info (not necessary valid).
       We assume that it is only used after index file was read */
    public boolean hasSigningInfo() {
        return (section4Length != 0);
    }

    /* only update if fully signed by common or single code source */
    public void updateValidationResultsForApplet(boolean state,
            Map /*<String, Long> */ trustedEntries, 
            long timestampt, long certExpiration) {
	
	Map map = null;
        if (Config.isJavaVersionAtLeast15()) {
	    map = getSignerMap();
	} else {
	    map = getCertificateMap();
	}
	if (map != null) {
	    /* Don't validate unless JAR complies with both applet and JNLP signing rules */
	    if (hasStrictSingleSigning()) {
        	Trace.println("updateValidationResultsForApplet update", TraceLevel.BASIC);
		updateValidationResults(state, trustedEntries, timestampt, certExpiration);
	    }
	}
    }
    
    public void updateValidationResults(boolean state,
            Map /*<String, Long> */ trustedEntries, 
            long timestampt, long certExpiration) {
        knownToBeSigned = state;
        validationTimestampt = timestampt;
        certExpirationDate = certExpiration;
        checkedJars = trustedEntries;
        Trace.println("Mark prevalidated: "+url+" "+state +
                " tm="+timestampt+" cert="+certExpiration,
                TraceLevel.CACHE);
        try {
            updateSecurityValidationCache();
            updateIndexHeaderOnDisk();
        } catch (IOException e) {
            Trace.println("Failed to update list of trusted cached entries in the index", TraceLevel.CACHE);
            Trace.ignoredException(e);
        }
    }

    private void updateSecurityValidationCache() throws IOException {
        try {
            AccessController.doPrivileged(new PrivilegedExceptionAction() {
                public Object run() throws IOException {
                    RandomAccessFile raf = null;
                    try {
                        raf = openLockIndexFile("rw", false);
                        section5Length = 0; //will update later if there will be no exceptions

                        if (checkedJars != null && !checkedJars.isEmpty()) {
                            ByteArrayOutputStream bos = new ByteArrayOutputStream(500);
                            DataOutputStream dos = new DataOutputStream(bos);

                            Set entries = checkedJars.entrySet();
                            dos.writeInt(entries.size());
                            Iterator i = entries.iterator();
                            while (i.hasNext()) {
                                Entry e = (Entry) i.next();
                                dos.writeUTF((String) e.getKey());
                                dos.writeLong(((Long) e.getValue()).longValue());
                            }
                            dos.close();
                            bos.close();

                            byte[] data = bos.toByteArray();

                            raf.seek(section1Length + section2Length + section3Length + section4Length);
                            raf.write(data);
                            section5Length = data.length;
                        }
                    } finally {
                        if (raf != null) {
                            doUpdateHeader(raf);
                            raf.close();
                        }
                    }
                    return null;
                }
            });
        } catch (PrivilegedActionException pae) {
            if (pae.getException() instanceof IOException) {
                throw (IOException) pae.getException();
            }
        }
    }

    private void readSecurityValidationCache() throws IOException {
        if (section5Length != 0) {
            try {
                AccessController.doPrivileged(new PrivilegedExceptionAction() {
                    public Object run() throws IOException {
                        byte data[] = new byte[section5Length];
                        RandomAccessFile raf = openLockIndexFile("rw", false);
                        try {
                            raf.seek(section1Length + section2Length + section3Length + section4Length);
                            raf.readFully(data);
                        } finally {
                            raf.close();
                        }

                        ByteArrayInputStream bis = new ByteArrayInputStream(data);
                        DataInputStream dis = new DataInputStream(bis);

                        int i = dis.readInt();
                        Map m = new HashMap();
                        while (i > 0) {
                            String entryname = dis.readUTF();
                            Long timestampt = new Long(dis.readLong());
                            m.put(entryname, timestampt);
                            i--;
                        }
                        checkedJars = m;
                        return null;
                    }
                });
            } catch (PrivilegedActionException pae) {
                if (pae.getException() instanceof IOException) {
                    throw (IOException) pae.getException();
                }
            }
        } else {
            checkedJars = null;
        }
    }

    public Map getCachedTrustedEntries() {
        if (section5Length == 0 && checkedJars == null)
            return null;

        if (checkedJars == null) {
            try {
              readSecurityValidationCache();
            } catch (IOException e) {
                Trace.println("Failed to read list of trusted cached entries from index", TraceLevel.CACHE);
                Trace.ignoredException(e);
            }
        }
        return checkedJars;
    }

    private void invalidateEntryDueToException(Throwable e) {
        //Corrupt idx file - mark it as not usable
        Trace.println("Invalidating entry url="+url+" file="+indexFile.getAbsolutePath());
        Trace.ignored(e);
        invalidateEntry();
    }

    private void invalidateEntry() {
        setIncomplete(1);
        try {
            updateIndexHeaderOnDisk();
        } catch (IOException ioe) {
            Trace.ignoredException(ioe);
        }
    }


    public CacheEntry(File idxFile) {
        String path = idxFile.getPath();
        filename = path.substring(0, path.length() - 4);
        indexFile = idxFile;
        indexFileSyncAccess = new SyncFileAccess(indexFile);
        tempDataFile = new File(filename + "-temp");
        // initialize the Cache Entry
        AccessController.doPrivileged(
                new PrivilegedAction() {
            public Object run() {
                try {
                    readIndexFile();
                } catch (Throwable e) {
                    //Corrupt idx file - mark it as not usable
                    invalidateEntryDueToException(e);
                }
                return null;
            }
        });
    }

    public void generateShortcutImage() throws IOException {
        if (getIsShortcutImage() == 0) {
            setIsShortcutImage(1);
            updateIndexHeaderOnDisk();
        }
    }
    
    /*
     * @return true if the issuer of <code>cert1</code> corresponds to the
     * subject (owner) of <code>cert2</code>, false otherwise.
     */
    private static boolean isIssuerOf(X509Certificate cert1,
            X509Certificate cert2) {
        Principal issuer = cert1.getIssuerDN();
        Principal subject = cert2.getSubjectDN();
        if (issuer.equals(subject))
            return true;
        return false;
    }
    
    /**
     * Open the index file, using a RandomAccessFile object with the given mode.
     *
     * The FileChannel will be locked in shared mode, if mode is "r" (read only),
     * otherwise it will be locked exclusively.
     *
     * Open and locking will be tried up to 9 times, while waiting 100ms 
     * after each trial.
     * 
     * @param mode the file mode, either "r" or "rw"
     *
     * @param privileged if true, the RandomAccessFile cstr is called within
     *                   an AccessController.doPrivileged() block.    
     *
     * @return the opened and locked index file RandomAccessFile instance
     *
     * @throws FileNotFoundException 
     *         index file could not be found
     * @throws IOException 
     *         open and lock failed more than 9 times
     */
    private RandomAccessFile openLockIndexFile(final String mode, boolean privileged) 
    throws IOException {
        SyncFileAccess.RandomAccessFileLock rafL = null;
        RandomAccessFile raf = null;
        try {
            /* in-place upgrade might be time consuming */
            rafL = indexFileSyncAccess.openLockRandomAccessFile(mode, 10000, privileged);
            raf  = ( rafL != null ) ? rafL.getRandomAccessFile() : new RandomAccessFile(indexFile, mode);
            return raf;
        } finally {
            if ( rafL != null ) {
                rafL.release();
            }
        }
    }

    /* This function reads index files from cache version 602 only
     * and is used to support in place upgrade of existing cache entry.
     * Eventually it will be removed as cache version 602 will 
     * be less used. */
    //Only used from readIndex and does not need own priviledged block
    private void readIndexFileOld() {
        final int oldCacheVersion = 602;
        RandomAccessFile raf = null;

        try {
            if (indexFile.exists()) {
                raf = openLockIndexFile("r", false);

                setBusy(raf.read());
                setIncomplete(raf.read());
                setCacheVersion(raf.readInt());

                if (getCacheVersion() == Cache.getCacheVersion() && isValidEntry()) {
                    //another thread/process fixed it for us while we wait for lock
                    //try to read again using regular approach
                    raf.close();
                    raf = null;
                    readIndexFile();
                    return;
                }

                // only continue to read if cache index version matches
                if (getCacheVersion() == oldCacheVersion && isValidEntry()) {
                    setForceUpdate(raf.read());
                    setNoHref(raf.read());
                    setIsShortcutImage(raf.read());
                    setContentLength(raf.readInt());
                    setLastModified(raf.readLong());
                    setExpirationDate(raf.readLong());
                    setVersion(raf.readUTF());
                    setURL(raf.readUTF());
                    setNamespaceID(raf.readUTF());

                    // check if resource file exists
                    File resource = new File(getResourceFilename());
                    // cache entry is incomplete if resource file not exists
                    if (resource.exists() == false) {
                        raf.close();
                        raf = null;
                        invalidateEntry();
                    }
                } else {
                    // cache index version mismatch
                    raf.close();
                    raf = null;
                    invalidateEntry();
                }
                // continue to read only if entry is not incomplete
                if (getIncomplete() == 0) {
                     // read header
                    readHeadersOld(raf);
                }
            }
        } catch (IOException ioe) {
            // Exception during index read, mark entry as incomplete
            if (raf != null) {
                try {
                    raf.close();
                    raf = null;
                } catch (IOException e) {
                    Trace.ignoredException(e);
                }
            }
            invalidateEntryDueToException(ioe);
        } finally {
            try {
                if (raf != null) {
                    raf.close();
                }
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            }
        }
    }

    //the only external call is call from initFrom and
    //it does have priviledged block
    private void readIndexFile() {
        RandomAccessFile raf = null;

        /* RandomAccessFile is not buffered, 
         * but we want to avoid performing tons of syscalls to read few bytes.
         * Use own buffers. */
        try {
            if (indexFile.exists()) {
                raf = openLockIndexFile("r", false);

                /* Read mandatory section 1 */
                byte[] header = new byte[section1Length];
                int n = raf.read(header);

                DataInputStream in = new DataInputStream(
                        new ByteArrayInputStream(header, 0, n));

                setBusy(in.readByte());
                setIncomplete(in.readByte());
                setCacheVersion(in.readInt());

                //try to upgrade if needed
                if (getCacheVersion() != Cache.getCacheVersion()) {
                    //attempt to upgrade in place if possible:
                    //try read index in old format and write it in the new
                    //If we fail then entry will be marked incomplete

                    //we do not need this here and will be using same file
                    raf.close();
                    raf = null;

                    Trace.println("Trying to upgrade in place " +
                            indexFile.getAbsolutePath(), TraceLevel.CACHE);
                    readIndexFileOld();
                    cacheVersion = Cache.getCacheVersion();

                    boolean isOK = isValidEntry();
                    try {
                        //update header now, to prevent concurrent access
                        setBusy(1);
                        updateIndexHeaderOnDisk();

                        if (isOK) {
                            writeFileToDisk(); //this will enable entry on success
                            Trace.println("Upgrade of entry done", TraceLevel.CACHE);
                        } else {
                            Trace.println("Upgrade of incomplete entry done", TraceLevel.CACHE);
                        }
                    } catch (IOException ioe) {
                        setBusy(0);
                        invalidateEntryDueToException(ioe);
                    }
                    //if we succeed then everything is read,
                    //if we failed then there is no sense to continue anyway
                    return;
                }

                //read rest of entry
                setForceUpdate(in.readByte());
                setNoHref(in.readByte());
                setIsShortcutImage(in.readByte());
                setContentLength(in.readInt());
                setLastModified(in.readLong());
                setExpirationDate(in.readLong());

                // only continue to read if cache index version matches
                // and resource file exists
                File resource = new File(getResourceFilename());
                if (resource.exists() == false) {
                    // cache index version mismatch or resource does not exist
                    setIncomplete(1);
                }
                //for incomplete entry there is no sense to read further
                if (getIncomplete() == 1) {
                    raf.close();
                    return;
                }

                validationTimestampt = in.readLong();
                knownToBeSigned = (in.readByte() == 1) ? true : false;

                section2Length = in.readInt();
                section3Length = in.readInt();
                section4Length = in.readInt();
                section5Length = in.readInt();

                blacklistValidationTime = in.readLong();
                certExpirationDate = in.readLong();
                
                classVerificationStatus = in.readByte();
                reducedManifestLength = in.readInt();

		section4Pre15Length = in.readInt();
        	hasOnlySignedEntries = (in.readByte() == 1);
        	hasSingleCodeSource = (in.readByte() == 1);
    		section4CertsLength = in.readInt();
    		section4SignersLength = in.readInt();
		hasMissingSignedEntries = (in.readByte() == 1);
                trustedLibrariesValidationTime = in.readLong();
                reducedManifest2Length = in.readInt();

                in.close();

                /* Read mandatory section 2 */
                if (section2Length > 0) {
                    header = new byte[section2Length];
                    raf.read(header);
                    in = new DataInputStream(new ByteArrayInputStream(header));
                    setVersion(in.readUTF());
                    setURL(in.readUTF());
                    setNamespaceID(in.readUTF());
                    setCodebaseIP(in.readUTF());

                    // read http headers
                    readHeaders(in);
                }
                //try to upgrade when enhanced JAR api is available
		if (hasSigningInfo() &&
			((section4Pre15Length == 0 && enhancedJarAccess) ||
                        BlackList.hasBeenModifiedSince(blacklistValidationTime) ||
                        TrustedLibraries.hasBeenModifiedSince(trustedLibrariesValidationTime)) &&
			(!Cache.isSystemCacheEntry(this) || Environment.isSystemCacheMode())) {

                    raf.close();
                    raf = null;

                    Trace.println("Trying to update in place " +
                            indexFile.getAbsolutePath(), TraceLevel.CACHE);
                    cacheVersion = Cache.getCacheVersion();

                    boolean isOK = isValidEntry();
                    try {
                        //update header now, to prevent concurrent access
                        setBusy(1);
                        updateIndexHeaderOnDisk();

                        if (isOK) {
                            Trace.println("Upgrade writing to disk for " + resource, TraceLevel.CACHE);
                            writeFileToDisk(); //this will enable entry on success
                            Trace.println("Upgrade of entry done", TraceLevel.CACHE);
                        } else {
                            Trace.println("Upgrade of incomplete entry done", TraceLevel.CACHE);
                        }
                    } catch (IOException ioe) {
                        setBusy(0);
                        invalidateEntryDueToException(ioe);
                    }
                    //if we succeed then everything is read,
                    //if we failed then there is no sense to continue anyway
                    Trace.println("readIndexFile returning success", TraceLevel.CACHE);
                    return;
                }
            }
        } catch (IOException ioe) {
            // Exception during index read, mark entry as incomplete
            setIncomplete(1);
            Trace.ignoredException(ioe);
        } finally {
            try {
                if (raf != null) {
                    raf.close();
                }
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            }
        }
    }
    
    public void setBusy(int busyFlag) {
        busy = busyFlag;
    }
    
    int getBusy() {
        return busy;
    }
    
    private void setCacheVersion(int version) {
        cacheVersion = version;
    }
    
    int getCacheVersion() {
        return cacheVersion;
    }
    
    public void setIncomplete(int incompleteFlag) {
        incomplete = incompleteFlag;
    }
    
    int getIncomplete() {
        return incomplete;
    }

    public boolean isValidEntry() {
        return (busy == 0 && incomplete == 0);
    }

    
    // 1 if cache resource is used as shortcut image; otherwise 0
    public void setIsShortcutImage(int shortcutImageFlag) {
        isShortcutImage = shortcutImageFlag;
    }
    
    // 1 if cache resource is used as shortcut image; otherwise 0
    public int getIsShortcutImage() {
        return isShortcutImage;
    }
    
    private void setForceUpdate(int forceUpdateFlag) {
        forceUpdate = forceUpdateFlag;
    }
    
    private int getForceUpdate() {
        return forceUpdate;
    }
    
    void setNoHref(int noHrefFlag) {
        noHref = noHrefFlag;
    }
    
    private int getNoHref() {
        return noHref;
    }
    
    // compare the passed in cache entry with the current entry and see
    // who should be clean-up first.
    // return true if current entry should be removed first
    boolean removeBefore(CacheEntry ce) {
    
        // incomplete entry are removed first
        if (getIncomplete() == 1) {
            return true;
        }
        if (ce.getIncomplete() == 1) {
            return false;
        }        
       
        // less recently used files are deleted before more recently used
        //    ones. the lastModified time of the resource index file indicates
        //    when is it last accessed
        long lastModified = getIndexFile().lastModified();
        long ceLastModified = ce.getIndexFile().lastModified();
        
        if (lastModified < ceLastModified) {
            // current entry is not accessed lately, remove it first
            return true;
        } else if (lastModified > ceLastModified){
            return false;
        }
      
        // If resources has the same lastAccessed time, expired resources
        //    are removed first
        long currentDate = System.currentTimeMillis();     
        long ce1Expired = getExpirationDate();
        long ce2Expired = ce.getExpirationDate();
        // check if they have different expiration date
        if (ce1Expired != ce2Expired) {
            if (ce1Expired < currentDate) {
                // current entry is expired, remove it first
                return true;
            }
            if (ce2Expired < currentDate) {
                // pass in entry is expired, remove that first
                return false;
            }
        }
      
        // larger resource should be removed before smaller ones
        if (getContentLength() >= ce.getContentLength()) {
            return true;
        }
        return false;
    }
    
    void setContentLength(int length) {
        contentLength = length;
    }
    
    public int getContentLength() {
        return contentLength;
    }
    
    public Map getCertificateMap() {
        Map ret = (signerMapCertRef != null) ?
            (Map) signerMapCertRef.get() : null;
        if (!doneReadCerts || (signerMapCertRef != null && ret == null)) {
            try {
                readCertificates();
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
                recover();
                try {
                  readCertificates();
                } catch (IOException e) {
                  //recovery did not help, can not do anything
                  invalidateEntryDueToException(e);
                }
            }
            ret = signerMapCertHardRef;
            clearHardRefs();
        } 
        touchRefs();
        return ret;
    }
    
    public Map getSignerMap() {
         Map ret = (signerMapRef != null) ? (Map) signerMapRef.get() : null;
         if (!doneReadSigners || (signerMapRef != null && ret == null)) {
            try {
                readSigners();
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
                recover();
                try {
                  readSigners();
                } catch (IOException e) {
                  //recovery did not help, can not do anything
                  invalidateEntryDueToException(e);
                }
            }
            ret = signerMapHardRef;
            clearHardRefs();
        } 
        touchRefs();
        return ret;
    }

    // Called from CachedJarFile and CachedJarFile14
    boolean hasSingleCodeSource() {
	return hasSingleCodeSource;
    }

    boolean hasStrictSingleSigning() {
	return hasOnlySignedEntries && hasSingleCodeSource && !hasMissingSignedEntries;
    }

    boolean hasOnlySignedEntries() {
	return hasOnlySignedEntries;
    }

    boolean hasMissingSignedEntries() {
	return hasMissingSignedEntries;
    }

    Map getCodeSourceCertCache() {
        Map codeSourceCertCache = (codeSourceCertCacheRef != null) ?
            (Map) codeSourceCertCacheRef.get() : null;
        if (!doneReadCerts ||
            (codeSourceCertCacheRef != null && codeSourceCertCache == null)) {
            try {
                readCertificates();
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
                recover();
                try {
                    readCertificates();
                } catch (IOException e) {
                    //recovery did not help, can not do anything
                    invalidateEntryDueToException(e);
                }
            }
            codeSourceCertCache = codeSourceCertCacheHardRef;
            clearHardRefs();
        }
        touchRefs();
	return codeSourceCertCache;
    }

    Map getCodeSourceCache() {
        if (Config.isJavaVersionAtLeast15()) {
            Map codeSourceCache = (codeSourceCacheRef != null) ?
                (Map) codeSourceCacheRef.get() : null;
            if (!doneReadSigners ||
                (codeSourceCacheRef != null && codeSourceCache == null)) {
                try {
                    readSigners();
                } catch (IOException ioe) {
                    Trace.ignoredException(ioe);
                    recover();
                    try {
                        readSigners();
                    } catch (IOException e) {
                        //recovery did not help, can not do anything
                        invalidateEntryDueToException(e);
                    }
                }
                codeSourceCache = codeSourceCacheHardRef;
                clearHardRefs();
            }
            touchRefs();
	    return codeSourceCache;
	} else {
	    return null;
	}
    }

    // Called from CachedJarFile and SigningInfo
    public CodeSigner[] getCodeSigners() {
        CodeSigner[] ret = (signersRef != null) ?
            (CodeSigner[]) signersRef.get() : null;
        if (!doneReadSigners ||
            (ret == null && signersRef != null)) {
            try {
                readSigners();
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
                recover();
                try {
                  readSigners();
                } catch (IOException e) {
                  //recovery did not help, can not do anything
                  invalidateEntryDueToException(e);
                }
            }
            ret = signersHardRef;
            clearHardRefs();
        }
        touchRefs();
        return ret;
    }
    
    public Certificate[] getCertificates() {
        Certificate[] ret = (certificatesRef != null) ?
            (Certificate[]) certificatesRef.get() : null;
        if (!doneReadCerts ||
            (certificatesRef != null && ret == null)) {
            try {
                readCertificates();
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
                recover();
                try {
                  readCertificates();
                } catch (IOException e) {
                  //recovery did not help, can not do anything
                  invalidateEntryDueToException(e);
                }
            }
            ret = certificatesHardRef;
            clearHardRefs();
        } 
        touchRefs();
        return ret;
    }

    // Called from CachedJarFile and CachedJarFile14
    CodeSource[] getCodeSources(URL url) {
	Collection codeSources = null;
        CodeSource[] ret = null;
        if (Config.isJavaVersionAtLeast15()) {
	    Map codeSourceCache = getCodeSourceCache();
	    if (codeSourceCache != null) {
	        codeSources = codeSourceCache.values();
	    }
        } else {
            Map codeSourceCertCache = getCodeSourceCertCache();
	    if (codeSourceCertCache != null) {
	        codeSources = codeSourceCertCache.values();
	    }
        }

	if (codeSources != null) {
	    int size = codeSources.size();
	    if (hasOnlySignedEntries) {
	        ret = (CodeSource[]) codeSources.toArray(new CodeSource[size]);
	    } else {
	        ret = (CodeSource[]) codeSources.toArray(new CodeSource[size+1]);
	        ret[size] = getUnsignedCS(url);
	    }
	} else {
	    ret = new CodeSource[] { getUnsignedCS(url) };
	}
        return ret;
    }

    private void touchRefs() {
        Object o;
        if (signerMapRef != null) o = signerMapRef.get();
        if (signersRef != null) o = signersRef.get();
        if (certificatesRef != null) o = certificatesRef.get();
        if (signerMapCertRef != null) o = signerMapCertRef.get();
        if (manifestRef != null) o = manifestRef.get();
        if (codeSourceCacheRef != null) o = codeSourceCacheRef.get();
        if (codeSourceCertCacheRef != null) o = codeSourceCertCacheRef.get();
    }
    
    private void clearHardRefs() {
        signerMapHardRef = null;
        signersHardRef = null;
        signerMapCertHardRef = null;
        certificatesHardRef = null;
	codeSourceCacheHardRef = null;
	codeSourceCertCacheHardRef = null;
    }
    
    void setLastModified(long time) {
        lastModified = time;
    }
    
    public long getLastModified() {
        return lastModified;
    }
    
    void setExpirationDate(long time) {
        expirationDate = time;
    }
    
    public void updateExpirationInIndexFile(long time) {
      
        if (getExpirationDate() == time) {
            // no need to update
            return;
        }
        setExpirationDate(time);

        try {
          updateIndexHeaderOnDisk();
        } catch (IOException ioe) {
          Trace.ignoredException(ioe);
        }
    }
    
    public long getExpirationDate() {
        return expirationDate;
    }
    
    public boolean isExpired() {
        if (expirationDate != 0) {
            if (System.currentTimeMillis() < expirationDate) {
                // cache entry not expired
                return false;
            }
        }
        // expired
        return true;
    }
    
    void setURL(String u) {
        url = u;
    }
    
    public String getURL() {
        return url;
    }
    
    void setVersion(String ver) {
        if (ver == null || ver.equals("")) {
            version = null;
        } else {
            version = ver;
        }
    }
    
    public String getVersion() {
        return version;
    }
    
    private void setNamespaceID(String id) {
        namespaceID = id;
    }
    
    private String getNamespaceID() {
        return namespaceID;
    }
    
    public JarFile getJarFile() {
        // need doPrivileged block here for the CachedJarFile object creation,
        // since this might be called directly from applet code
        JarFile jf = (JarFile) AccessController.doPrivileged(
                new PrivilegedAction() {
            public Object run() {
                if (Config.isJavaVersionAtLeast15()) {
                    //creating CachedJarFile is cheap and needed 
                    // to keep track of usage counter 
                    CachedJarFile jar = null; 
                    try {
                       jar = new CachedJarFile(CacheEntry.this);
                    } catch (IOException ioe) {
                            Trace.ignoredException(ioe);
                    }
                    return jar;
                } else {
                    //creating CachedJarFile is cheap and needed 
                    // to keep track of usage counter 
                    CachedJarFile14 jar = null; 
                    try {
                       jar = new CachedJarFile14(CacheEntry.this);
                    } catch (IOException ioe) {
                            Trace.ignoredException(ioe);
                    }
                    return jar;
                }
            }
        });
        return jf;
    }
    
    public String getResourceFilename() {
        return filename;
    }
    
    public File getDataFile() {
        File f = null; 
        if (filename != null && url.equals("") == false) {
             f = new File(filename);
             MemoryCache.addResourceReference(f, url);
        }
        return f;
    }
    
    File getTempDataFile() {
        return tempDataFile;
    }    
    
    public File getIndexFile() {
        return indexFile;
    }

    public long getSize() {
        return getDataFile().length() + getIndexFile().length();
    }
    
    void setHeaders(MessageHeader headers) {
        headerFields = headers;
    }
    
    public Map getHeaders() {
        return headerFields.getHeaders();
    }
    
    public boolean isHttpNoCacheEnabled() {
        
        // HTTP 1.1
        String cacheControlValue = headerFields.getValue(headerFields.getKey("cache-control"));
        
        if (cacheControlValue != null && cacheControlValue.equals("no-cache")) {
            return true;
        }
        
        // HTTP 1.0
        String pragmaValue = headerFields.getValue(headerFields.getKey("pragma"));
        
        if (pragmaValue != null && pragmaValue.equals("no-cache")) {
            return true;
        }
      
        return false;
    }
    
    boolean processTempDataFile(boolean applyJarDiff,
            DownloadEngine.DownloadDelegate dd, URL href, URL requestURL, 
            String newVersion) {
        boolean ret = false;
        if (applyJarDiff) {
            String currentVersion = null;
            String query = requestURL.getQuery().toString();
            StringTokenizer st = new StringTokenizer(query, "&");
            String tokens = null;
            while (st.hasMoreTokens()) {
                tokens = st.nextToken();
                if (tokens.startsWith(
                        DownloadEngine.ARG_CURRENT_VERSION_ID)) {
                    currentVersion = tokens.substring(
                            DownloadEngine.ARG_CURRENT_VERSION_ID.length()
                            + 1);
                    break;
                }
            }
            CacheEntry currentCE = null;
                
            currentCE = Cache.getCacheEntry(href, null, currentVersion);
            
            File currentFile = null;
            if (currentCE != null) {
                currentFile = new File(currentCE.getResourceFilename());
            }
            File newFile = null;
            try {
                if (Trace.isTraceLevelEnabled(TraceLevel.NETWORK)) {
                    Trace.println(ResourceManager.getString(
                        "cacheEntry.applyJarDiff", href == null ? 
                            "" : href.toString(), currentVersion, 
                        newVersion), TraceLevel.NETWORK);
                }
                
                newFile = DownloadEngine.applyPatch(currentFile,
                        tempDataFile, href, newVersion, dd, filename);
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
            }
            AccessController.doPrivileged(
                    new PrivilegedAction() {
                        public Object run() {
                            tempDataFile.delete();
                            return null;
                        }
                    });
            if (newFile != null) {
                ret = true;
            }
        }
        Boolean b = (Boolean) AccessController.doPrivileged(
                new PrivilegedAction() {
                    public Object run() {
                        return new Boolean(tempDataFile.renameTo(new File(filename)));
                    }
                });
        if (b.booleanValue()) {
            ret = true;
        }
        return ret;
    }

    //Copy content of given cache entry into this object.
    //We could simply copy all fields here but given number of fields
    //and frequent changes in the set of fields
    //it is safer to read entry from disk cache.
    //This method is supposed to be used for recovery only and
    // performance is not the issue here
    private void initFrom(CacheEntry ce) {
        Trace.println("Recovering CacheEntry for "+ce.getURL(), TraceLevel.CACHE);
        filename = ce.filename;
        indexFile = ce.indexFile;
        indexFileSyncAccess = new SyncFileAccess(indexFile);
        tempDataFile = new File(filename + "-temp");
        // initialize the Cache Entry
        AccessController.doPrivileged(
                new PrivilegedAction() {
            public Object run() {
                readIndexFile();
                return null;
            }
        });
    }

    
    /* Attempt to recover current cache entry object.
     * 
     * If all fails - remove cache entry and throw RuntimeException
     */
    private void recover() {
        Trace.println("Trying to recover cache entry for "+url, TraceLevel.CACHE);
        try {
            URL u = new URL(url);
            String v = getVersion();
            String id = getNamespaceID();

            //reload
            //remove from memory cache first or it will prevent removal on disk
            MemoryCache.removeLoadedResource(url);
            //keep LAP file
            Cache.removeCacheEntry(this, false);
            DownloadEngine.getCachedFile(u);
            CacheEntry ce = Cache.getCacheEntry(u, id, v);
            if (ce != null) {
                initFrom(ce);
            } else {
                throw new RuntimeException("ERROR: Recovery got null entry");
            }
        } catch (Exception e) {
            throw new RuntimeException("ERROR: Failed to recover corrupt cache entry");
        }
    }

    public Manifest getManifest() {
        Manifest ret = (manifestRef != null) ? (Manifest) manifestRef.get() : null;
        if (!doneReadManifest || (manifestRef != null && ret == null)) {
            try {
                ret = readManifest();
            } catch (IOException ioe) {
                Trace.ignoredException(ioe);
                recover();
                try {
                  ret = readManifest();
                } catch (IOException e) {
                  //recovery did not help, can not do anything
                  invalidateEntryDueToException(e);
                }
            } finally {
                clearHardRefs();
            }
        }
        touchRefs();
        return ret;
    }
    
    private void setCodebaseIP(String ipaddr) {
        codebaseIP = ipaddr;
    }
    
    public String getCodebaseIP() {
        return codebaseIP;
    }
    
    public void writeFileToDisk() throws IOException {
        writeFileToDisk(DownloadEngine.NORMAL_CONTENT_BIT, null);
    }
    
    private boolean hasMimeType(String mimeType) {
      if (headerFields != null) {
          Object o = getHeaders().get("content-type");
          // Be robust
          if (!(o instanceof List))
              return false;
          List l = (List) o;
          return l.contains(mimeType);
      }
      return false;
    }
    
    private boolean hasRequestType(String mimeType) {
      if (headerFields != null) {
          Object o = getHeaders().get(HttpRequest.DEPLOY_REQUEST_CONTENT_TYPE);
          // Be robust
          if (!(o instanceof List))
              return false;
          List l = (List) o;
          return l.contains(mimeType);
      }
      return false;
    }


    public boolean isJarFile(String filename) {
        int coln;
    
        if (hasRequestType(HttpRequest.JAR_MIME_TYPE)) {
            return true;
        }
        
        if ((coln = filename.indexOf(";")) != -1) {            
            filename = filename.substring(0, coln);
        }
        if ((coln = filename.indexOf("?")) != -1) {            
            filename = filename.substring(0, coln);
        }
        return filename.toLowerCase().endsWith(".jar") || 
               filename.toLowerCase().endsWith(".jarjar") ||
               hasMimeType(HttpRequest.JAR_MIME_TYPE) ||
               hasMimeType(HttpRequest.JAR_MIME_TYPE_EX) ||
               hasMimeType(HttpRequest.JARDIFF_MIME_TYPE);
    }
    
    public boolean isJNLPFile() {
        int coln;

        if (hasRequestType(HttpRequest.JNLP_MIME_TYPE)) {
            return true;
        }

        String filename = url;
        if ((coln = filename.indexOf(";")) != -1) {
            filename = filename.substring(0, coln);
        }
        if ((coln = filename.indexOf("?")) != -1) {
            filename = filename.substring(0, coln);
        }
        return (filename.toLowerCase().endsWith(".jnlp") ||
                filename.toLowerCase().endsWith(".jarjnlp"));
    }

    /* sync header of the index file to the disk
       (assuming that rest of index file is already there).
       IMPORTANT: if you are obtaining file locks in your code 
          then make sure to release them or use doUpdateHeader() 
          as locks are not preemptive. */
    public void updateIndexHeaderOnDisk() throws IOException {
        try {
            AccessController.doPrivileged(new PrivilegedExceptionAction() {

                public Object run() throws IOException {
                    RandomAccessFile raf = null;
                    try {
                        raf = openLockIndexFile("rw", false);
                        doUpdateHeader(raf);
                        // output header of index file to disk
                    } finally {
                        if (raf != null) {
                            raf.close();
                        }
                    }
                    return null;
                }
            });
        } catch (PrivilegedActionException pae) {
            if (pae.getException() instanceof IOException) {
                throw (IOException) pae.getException();
            }
        }
    }

    //Use this to update header on the disk 
    //if we already have write lock
    //NB: callee is responsible to run this in priviledged block
    private void doUpdateHeader(RandomAccessFile raf) throws IOException {
        if (raf != null) {
            raf.seek(0);
            raf.write(prepareHeader());
        }
    }

    private byte[] prepareHeader() throws IOException {
        ByteArrayOutputStream headerdata = new ByteArrayOutputStream(section1Length);
        DataOutputStream s = new DataOutputStream(headerdata);
        s.writeByte(busy);
        s.writeByte(incomplete);
        s.writeInt(cacheVersion); //3-6
        s.writeByte(forceUpdate);
        s.writeByte(noHref);
        s.writeByte(isShortcutImage);
        s.writeInt(contentLength); //10-13
        s.writeLong(lastModified); //14-
        s.writeLong(expirationDate); //22-

        s.writeLong(validationTimestampt); // 30-
        s.writeByte(knownToBeSigned ? 1 : 0); //38-38
        s.writeInt(section2Length); //39-
        s.writeInt(section3Length); //43-
        s.writeInt(section4Length); //47-
        s.writeInt(section5Length); //51-

        s.writeLong(blacklistValidationTime); //55-
        s.writeLong(certExpirationDate); // 63-
        
        s.writeByte(classVerificationStatus);
        s.writeInt(reducedManifestLength); //71 -

	s.writeInt(section4Pre15Length); // 71-
        s.writeByte(hasOnlySignedEntries ? 1 : 0); // 75-
        s.writeByte(hasSingleCodeSource ? 1 : 0); // 76-
    	s.writeInt(section4CertsLength); // 77-
    	s.writeInt(section4SignersLength); // 81-
        s.writeByte(hasMissingSignedEntries ? 1 : 0); // 85-
        s.writeLong(trustedLibrariesValidationTime); //89-
        s.writeInt(reducedManifest2Length); //97 -

	s.flush();
        if (headerdata.size() < section1Length) {
           //ensure header fully fills reserved space
           byte junk[] = new byte[section1Length - headerdata.size()];
           s.write(junk);
        }
        s.close();
        return headerdata.toByteArray();
    }

    // This is used from Cache class too. Make sure we are in priviledged block here
    void writeFileToDisk(final int contentType, final DownloadEngine.DownloadDelegate dd)
            throws IOException {
        try {
            AccessController.doPrivileged(new PrivilegedExceptionAction() {
                public Object run() throws IOException {
                    JarFile jar = null;
                    RandomAccessFile raf = null;

                    //reset lengths as they will be updated
                    //and they could be stale (e.g. if we are upgrading from old index file)
                    section2Length = 0;
                    section3Length = 0;
                    section4Length = 0;
		    section4Pre15Length = 0;
    		    section4CertsLength = 0;
    		    section4SignersLength = 0;
                    section5Length = 0;
                    reducedManifestLength = 0;
                    reducedManifest2Length = 0;

                    try {
                        raf = openLockIndexFile("rw", false);
                        // output index file contents to disk

                        //mandatory header first (will write it again later on)
                        byte header[] = prepareHeader();
                        raf.write(header);

                        ByteArrayOutputStream bout = new ByteArrayOutputStream(1000);
                        DataOutputStream out = new DataOutputStream(bout);

                        out.writeUTF(getVersion() != null ? getVersion() : "");
                        out.writeUTF(getURL());
                        out.writeUTF(getNamespaceID());

                        // write out resource codebase ip address if available
                        InetAddress ina = null;

                        // get the ip address of the resource codebase
                        String codebase = "";
                        if (url != null && url.equals("") == false) {
                            URL u = new URL(url);
                            String host = u.getHost();
                            ina = Cache.getHostIP(host);
                            if (ina != null) {
                                codebase = ina.getHostAddress();
                            }
                        }
                        out.writeUTF(codebase);

                        // write out HTTP/HTTPS header if available
                        writeHeaders(out);

                        out.close();
                        bout.close();
                        section2Length = bout.size();
                        raf.write(bout.toByteArray());

                        if (incomplete == 0) {
                            // save sections 3 and 4 (JAR only)
                            if (isJarFile(url)) {
                                jar = new JarFile(new File(filename));
                                CachedManifest manifest = new CachedManifest(jar);
                                //will update section3Length and section4Length internally
                                writeManifest(raf, jar, manifest, contentType, dd);
                                manifest.postprocess(); //need to do this explicilty
                                updateManifestRefs(manifest);
                                jar.close();
                            }
                            // this entry just got downloaded, so mark it as update check done
                            DownloadEngine.addToUpdateCheckDoneList(url);

                            // add this entry to the cleanup thread loaded resource list
                            Cache.addToCleanupThreadLoadedResourceList(url);

                            setBusy(0);
                            setIncomplete(0);
                            updateBlacklistValidation();
                            updateTrustedLibrariesValidation();
                            doUpdateHeader(raf);

                            //whenether this is jar or not we do not need to try read
                            //manifest or certificates again
                            doneReadManifest = true;
                            doneReadCerts = true;
                            doneReadSigners = true;
                        }
                    } catch (Exception e) {
                        Trace.ignoredException(e);
                        // close file before trying to delete them
                        // set raf/jar to null after closing, so they won't be closed
                        // again in the finally block
                        if (raf != null) {
                            raf.close();
                            raf = null;
                        }
                        if (jar != null) {
                            jar.close();
                            jar = null;
                        }

                        Cache.removeCacheEntry(CacheEntry.this);
                        if (e instanceof JARSigningException) {
                            throw (JARSigningException) e;
                        }
                        if (e instanceof java.util.zip.ZipException) {
                            throw new JARSigningException(new URL(url), version,
                                    JARSigningException.BAD_SIGNING, e);
                        }
                        throw new IOException(e.getMessage());
                    } finally {
                        if (raf != null) {
                            raf.close();
                        }
                        if (jar != null) {
                            jar.close();
                        }
                        Cache.cleanup();
                    }
                    return null;
                }
            });
        } catch (PrivilegedActionException pae) {
            if (pae.getException() instanceof IOException) {
                throw (IOException) pae.getException();
            }
        }
    }

    private void updateManifestRefs(Manifest jarManifest) {
        if (jarManifest != null) {
            manifestRef = new SoftReference(jarManifest);
        } else {
            manifestRef = null;
        }
    }

    private byte[] readBlock(final int offset, final int length) throws IOException {
        try {
            return (byte[]) AccessController.doPrivileged(new PrivilegedExceptionAction() {

                public Object run() throws IOException {
                    RandomAccessFile raf = openLockIndexFile("r", false);
                    try {
                        raf.seek(offset);
                        BufferedInputStream in = null;
                        byte data[] = new byte[length];
                        raf.readFully(data);
                        return data;
                    } finally {
                        if (raf != null) {
                            raf.close();
                        }
                    }
                }
            });
        } catch (PrivilegedActionException pae) {
            if (pae.getException() instanceof IOException) {
                throw (IOException) pae.getException();
            }
            return null;
        }
    }
    
    /*
     * In 6u18 the reduced manifest, if present, is first in section 3 and is
     * followed by the full manifest. This is binary incompatible with 6u14
     * through 6u17. Cache indices written by 6u19 and later switch the ordering.
     * Note that 6u14 through 6u17 will read the entire section 3 block but
     * only the full manifest data will be parsed.
     */
    byte[] getFullManifestBytes() throws IOException {
        if (reducedManifest2Length > 0) { // have simplified 6u19+ version of manifest
	    int offset = section1Length + section2Length;
            int length = section3Length - reducedManifest2Length;
            return readBlock(offset, length);
        } else if (reducedManifestLength > 0) { // have simplified 6u18 version of manifest
	    int offset = section1Length + section2Length + reducedManifestLength;
            int length = section3Length - reducedManifestLength;
            return readBlock(offset, length);
	}
        return null;
    }

    Manifest readManifest() throws IOException {
        CachedManifest ret = null;
        if (section3Length != 0) {
            int length;
            int offset = section1Length + section2Length;
            if (reducedManifest2Length > 0) { //have 6u19+ simplified version of manifest
		// have 6u19+ simplified version of manifest
                offset += (section3Length - reducedManifest2Length);
                length = reducedManifest2Length;
            } else if (reducedManifestLength > 0) { // have 6u18 simplified version of manifest
                length = reducedManifestLength;
	    } else {
                length = section3Length;
            }
            byte data[] = readBlock(offset, length);
            Trace.println(" Read manifest for " +url 
                    + ": read=" + length
                    + " full=" + section3Length,
                    TraceLevel.CACHE);
            ret = new CachedManifest(this.getURL(), data,
                    (reducedManifest2Length > 0 || reducedManifestLength > 0));
            updateManifestRefs(ret);
        }
        doneReadManifest = true;
        return ret;
    }

    private void readCertificates() throws IOException {
	final int sectionLength = (section4Pre15Length != 0) ? section4Pre15Length : section4Length;
	final int certsLength = (hasStrictSingleSigning()) ?
			   section4CertsLength : sectionLength;
	Trace.println("Reading certificates from " + certsLength + " " +
		url + " | " + indexFile.getAbsolutePath(), TraceLevel.CACHE);
	if (section4Length != 0) {
	    try {
		AccessController.doPrivileged(new PrivilegedExceptionAction() {
		    public Object run() throws IOException {
			RandomAccessFile raf = openLockIndexFile("r", false);
			try {
			    raf.seek(section1Length + section2Length + section3Length);
			    byte[] data = new byte[certsLength];
                            raf.readFully(data);
                            ByteArrayInputStream bin = new ByteArrayInputStream(data);
                            BufferedInputStream buf = new BufferedInputStream(bin);
        		    ObjectInputStream in = new 
                                    IndexFileObjectInputStream(buf);
        		    BufferedReader reader = new BufferedReader(new InputStreamReader(in));
                            readCertificates(in, reader);
                            doneReadCerts = true;
                        } finally {
                            if (raf != null) {
                                raf.close();
                            }
                        }
                        return null;
                    }
                });
            } catch (PrivilegedActionException pae) {
                if (pae.getException() instanceof IOException) {
                    throw (IOException) pae.getException();
                }
            }
        }
    }


    /*
     * Read in the CodeSigners on a 1.5+ JRE. This is a bit convoluted since
     * the index file may have been generated on a pre-1.5 JRE and thus have
     * no actual CodeSigners in which case we read the certificate data
     * and convert it to CodeSigners. We must also distinguish index files
     * written by older JREs in which some newer section 1 flags are
     * not supported and are defaulted to 0.
     */
    private void readSigners() throws IOException {
	final boolean processCertificates = section4Pre15Length != 0 && section4SignersLength < 5;
	final int certsLength = (section4Pre15Length != 0 && !(processCertificates && !hasStrictSingleSigning())) ?
				 section4CertsLength : section4Length - (section4Length - section4Pre15Length);
	final int signersLength = (hasStrictSingleSigning() || processCertificates) ?
				   section4SignersLength : section4Length - section4Pre15Length;
        Trace.println("Reading Signers from " + signersLength + " " +
                url + " | " + indexFile.getAbsolutePath(), TraceLevel.CACHE);
        if (section4Length != 0) {
            try {
                AccessController.doPrivileged(new PrivilegedExceptionAction() {
                    public Object run() throws IOException {
                        RandomAccessFile raf = openLockIndexFile("r", false);
                        try {
			    /*
			     * Concatenate the serialized form of the pre-1.5 section 4
			     * certificates together with the serialized form of the
			     * code signers to create a valid serialization stream with
			     * the pre-1.5 signed entry name and indices fields removed.
			     */
                            byte[] data = new byte[certsLength + signersLength];
			    if (certsLength != 0) {
				/*
				 * Read in partial or full certificates section from
				 * 6u19 index files. Older files will
				 * have certsLength == 0 and signersLength == section4Length.
				 */
			        raf.seek(section1Length + section2Length + section3Length);
                                raf.readFully(data, 0, certsLength);
			    }
                            raf.seek(section1Length + section2Length + section3Length + section4Pre15Length);
                            raf.readFully(data, certsLength, signersLength);
                            ByteArrayInputStream bin = new ByteArrayInputStream(data);
                            BufferedInputStream buf = new BufferedInputStream(bin);
        		    ObjectInputStream in = new 
                                    IndexFileObjectInputStream(buf);
        		    BufferedReader reader = new BufferedReader(new InputStreamReader(in));
			    if (signersLength == section4Length || processCertificates) {
				/* Process the certificates */
                                readCertificates(in, reader);
                                doneReadCerts = true;
			    } else {
				/* Eat and flush prior certificate data in serialization stream */
				int numCerts = in.readInt();
				try {
                		    for (int i=0; i < numCerts; i++) {
                		        /* foosh */ in.readObject();
                		    }
				} catch (ClassNotFoundException e) {
				    throw new IOException("Error reading signer certificates");
			        }
			    }
                            readSigners(in, reader);
                            doneReadSigners = true;
                        } finally {
                            if (raf != null) {
                                raf.close();
                            }
                        }
                        return null;
                    }
                });
            } catch (PrivilegedActionException pae) {
                if (pae.getException() instanceof IOException) {
                    throw (IOException) pae.getException();
                }
            }
        }
    }

    // caller of this method should handle closing of the BufferedInputStream
    // NB: assuming we are at first byte of section 4
    // NB: used from readCertificates() only and wrapped by priviledged action there
    private void readCertificates(ObjectInputStream in, BufferedReader reader) throws IOException {
        // Read the certificate array
        int numCerts = in.readInt();
        
        if (numCerts > 0) {
            Map signerMapCert = new HashMap();
	    Map codeSourceCertCache = new HashMap();
            Certificate certificates[] = new Certificate[numCerts];

            signerMapCertHardRef = signerMapCert;
            signerMapCertRef = new SoftReference(signerMapCertHardRef);
            
	    codeSourceCertCacheHardRef = codeSourceCertCache;
	    codeSourceCertCacheRef = new SoftReference(codeSourceCertCacheHardRef);

            certificatesHardRef = certificates;
            certificatesRef = new SoftReference(certificatesHardRef);

            try {
                for (int i = 0; i < numCerts; i++) {
                    certificates[i] = (Certificate)in.readObject();
                }
            } catch (ClassNotFoundException e) {
                throw new IOException("Error reading signer certificates");
            }
            
	    int[] signerIndicesCert = null;
	    URL u = new URL(url);
	    if (hasStrictSingleSigning()) {
		signerIndicesCert = new int[certificates.length];
		for (int i=0; i < certificates.length; i++) {
		    signerIndicesCert[i] = i;
		}
		CodeSource codeSource = new CodeSource(u, certificates);
                signerMapCert.put(null, signerIndicesCert);
		codeSourceCertCache.put(signerIndicesCert, codeSource);
		return;
	    }

            // Read the certificate signer map
            String certLine = reader.readLine();
            String lastPackageCert = null;
	    Map signerIndicesCertCache = new HashMap();
	    int[] singleSignerIndicesCert = null;
            
            while ((certLine !=null) && (!certLine.equals(""))) {
                // Read the entry name
                String certName = certLine;
                if (certName.startsWith("/")) {
                    certName = lastPackageCert + certName;
                } else {
                    int lastSlashCert = certName.lastIndexOf("/");
                    if (lastSlashCert != -1) {
                        lastPackageCert = certName.substring(0, lastSlashCert);
                    }
                }
                certLine = reader.readLine();
		if (singleSignerIndicesCert != null) {
                    signerMapCert.put(certName, singleSignerIndicesCert);
		} else {
		    signerIndicesCert = (int[]) signerIndicesCertCache.get(certLine);
		    if (signerIndicesCert == null) {
                        StringTokenizer tokenizer = new StringTokenizer(certLine, " ",
                            false);
                        int numEntryCerts = Integer.parseInt(tokenizer.nextToken());
                        signerIndicesCert = new int[numEntryCerts];
                        for (int i=0; i< numEntryCerts; i++) {
                            signerIndicesCert[i] = Integer.parseInt(
                                tokenizer.nextToken());
                        }
		        signerIndicesCertCache.put(certLine, signerIndicesCert);
		        Certificate[] certs = new Certificate[signerIndicesCert.length];
		        for (int i = 0; i < signerIndicesCert.length; i++) {
		            certs[i] = certificates[signerIndicesCert[i]];
		        }
		        CodeSource codeSource = new CodeSource(u, certs);
		        codeSourceCertCache.put(signerIndicesCert, codeSource);
		    }
                    signerMapCert.put(certName, signerIndicesCert);
		    if (hasSingleCodeSource) {
			singleSignerIndicesCert = signerIndicesCert;
		    }
		}
                certLine = reader.readLine();
            }
        } else {
            Trace.println(ResourceManager.getString("cacheEntry.unsignedJar",
                    url), TraceLevel.NETWORK);
        }
    }
            
    // caller of this method should handle closing of the BufferedInputStream
    // NB: assuming we are at first byte of section 4
    // NB: used from readCertificates() only and wrapped by priviledged action there
    private void readSigners(ObjectInputStream in, BufferedReader reader) throws IOException {

        // Read codeSigner object info
        String numCSStr = reader.readLine();
        int numCS = 0;

        // Read more codeSigner object info if we are running
        // JRE 1.5 or above
        if (!Config.isJavaVersionAtLeast15()) {
	    /*
	     * This should never happen. Calls to here only originate
	     * from CachedJarFile which is only loaded on 1.5+.
	     */
	    Trace.println("readSigners called pre-1.5 ", TraceLevel.CACHE);
	    return;
	}

	if (numCSStr != null) {
	    numCS = Integer.parseInt(numCSStr);
	}
        Map signerMap = new HashMap();
	Map codeSourceCache = new HashMap();

	if (numCS == 0) {
	    if (signerMapCertHardRef == null || codeSourceCertCacheHardRef == null) {
                Trace.println(ResourceManager.getString("cacheEntry.unsignedJar",
                    url), TraceLevel.NETWORK);
		return;
	    }

            signerMapHardRef = signerMap;
            signerMapRef = new SoftReference(signerMapHardRef);

	    codeSourceCacheHardRef = codeSourceCache;
	    codeSourceCacheRef = new SoftReference(codeSourceCacheHardRef);

	    /*
	     * Use CodeSource to convert certificate-based signers
	     * to CodeSigner signers. Map certificate-based indices
	     * to code signer indices.
	     */
            Map signerMapCert = signerMapCertHardRef;
	    Map codeSourceCertCache = codeSourceCertCacheHardRef;
	    List signersCS = new ArrayList();
	    Map indicesMap = new HashMap();
	    Iterator itor = codeSourceCertCache.entrySet().iterator();
	    while (itor.hasNext()) {
	        CodeSource cs;
	        CodeSigner[] entrySigners;
	        int[] signerIndicesCert;
	        Map.Entry pair;

	        pair = (Map.Entry) itor.next();
	        signerIndicesCert = (int[]) pair.getKey();
	        cs = (CodeSource)pair.getValue();
	        entrySigners = cs.getCodeSigners();

		/*
		 * Workaround poor j2se implementation of getSigners() for
		 * CodeSources constructed from Certificates.
		 */
		entrySigners = convertCertArrayToSignerArray(cs.getCertificates());
		cs = new CodeSource(cs.getLocation(), entrySigners);

                if ((entrySigners != null) && (entrySigners.length > 0)) {
                    int[] signerIndicesCS = new int[entrySigners.length];
                    for (int i = 0; i < entrySigners.length; i++) {
                        // Add the entry signer to the list of signers for this
                        // Jar file
                        int signerIndexCS = signersCS.indexOf(entrySigners[i]);
                        if (signerIndexCS == -1) {
                            signerIndexCS = signersCS.size();
                            signersCS.add(entrySigners[i]);
                        }
                        signerIndicesCS[i] = signerIndexCS;
                    }
                    indicesMap.put(signerIndicesCert, signerIndicesCS);
		    codeSourceCache.put(signerIndicesCS, cs);
                } // else runtime error
	    }

	    /*
	     * Convert certificate-based name map to code signer name map.
	     */
	    itor = signerMapCert.entrySet().iterator();
	    while (itor.hasNext()) {
	        Map.Entry pair;
		String name;
	        int[] signerIndicesCert;

	        pair = (Map.Entry) itor.next();
	        name = (String) pair.getKey();
	        signerIndicesCert = (int[]) pair.getValue();
	        signerMap.put(name, indicesMap.get(signerIndicesCert));
	    }


	    signersHardRef = (CodeSigner[]) signersCS.toArray(new CodeSigner[signersCS.size()]);
	    signersRef = new SoftReference(signersHardRef);

        } else { // We do find codeSigner info
            if (numCS > 0) {
                signerMapHardRef = signerMap;
                signerMapRef = new SoftReference(signerMapHardRef);

	        codeSourceCacheHardRef = codeSourceCache;
	        codeSourceCacheRef = new SoftReference(codeSourceCacheHardRef);

                CodeSigner signers[] = new CodeSigner[numCS];
                signersHardRef = signers;
                signersRef = new SoftReference(signersHardRef);
                try {
                    for (int i = 0; i < numCS; i++) {
			signers[i] = newCodeSigner((CodeSigner)in.readObject());
                    }
                } catch (ClassNotFoundException e) {
                    throw new IOException("Error reading code signer");
                }

		int[] signerIndices = null;
		URL u = new URL(url);
	        if (hasStrictSingleSigning()) {
		    signerIndices = new int[signers.length];
		    for (int i=0; i < signers.length; i++) {
		        signerIndices[i] = i;
		    }
		    CodeSource codeSource = new CodeSource(u, signers);
                    signerMap.put(null, signerIndices);
		    codeSourceCache.put(signerIndices, codeSource);
		    return;
	        }
                        
                // Read the codeSigner map
                String line = reader.readLine();
                String lastPackage = null;
		Map signerIndicesCache = new HashMap();

                while ((line != null) && (!line.equals(""))) {
                    // Read the entry name
                    String name = line;
                    if (name.startsWith("/")) {
                        name = lastPackage + name;
                    } else {
                        int lastSlash = name.lastIndexOf("/");
                        if (lastSlash != -1) {
                            lastPackage = name.substring(0, lastSlash);
                        }
                    }
                    line = reader.readLine();

		    signerIndices = (int[]) signerIndicesCache.get(line);
		    if (signerIndices == null) {
                        StringTokenizer tokenizer =
                            new StringTokenizer(line, " ", false);
                        int numEntrySigners = Integer.parseInt(
                            tokenizer.nextToken());
                        signerIndices = new int[numEntrySigners];
                        for (int i = 0; i < numEntrySigners; i++) {
                            signerIndices[i] = Integer.parseInt(tokenizer.nextToken());
                        }
			signerIndicesCache.put(line, signerIndices);
			CodeSigner[] codeSigners = new CodeSigner[signerIndices.length];
			for (int i = 0; i < signerIndices.length; i++) {
			    codeSigners[i] = signers[signerIndices[i]];
			}
			CodeSource codeSource = new CodeSource(u, codeSigners);
			codeSourceCache.put(signerIndices, codeSource);
		    }
                    signerMap.put(name, signerIndices);
                    line = reader.readLine();
	        }
	    }
	}
    }

    /*
     * Convert an array of certificates to an array of code signers.
     * The array of certificates is a concatenation of certificate chains
     * where the initial certificate in each chain is the end-entity cert.
     *
     * This is a workaround for a low-quality implementation in j2se.
     *
     * @return An array of code signers or null if none are generated.
     */
    private CodeSigner[] convertCertArrayToSignerArray(
 	Certificate[] certificates) throws IOException {

        try {
            // Now we have to break certificates chain and create
            // codeSigner object
            CertificateFactory cf = CertificateFactory.getInstance(
                    "X.509");
            ArrayList cpList = new ArrayList();
            
            int chainNum = 0;
            int start = 0;
            int end = 0;
            while (end < certificates.length) {
                ArrayList certList = new ArrayList();
                int i = start;
                for (i = start; i < certificates.length; i++) {
                    X509Certificate currentCert = null;
                    X509Certificate issuerCert = null;
                    
                    if (certificates[i] instanceof X509Certificate) 
                    {
                        currentCert = (X509Certificate)
                        certificates[i];
                    }
                    if (((i+1)<certificates.length) &&
                            certificates[i+1] instanceof
                            X509Certificate) {
                        issuerCert = (X509Certificate)
                        certificates[i+1];
                    } else {
                        issuerCert = currentCert;
                    }
                    certList.add(currentCert);
                    if (!isIssuerOf(currentCert, issuerCert)) {
                        break;
                    }
                }
                end = (i < certificates.length) ? (i + 1): i;
                
                // Create CertPath list
                CertPath cp = cf.generateCertPath(certList);
                certList.clear();
                cpList.add(cp);
                
                start = end;
                chainNum++;
            }
            
            // Create codeSigner object
            CodeSigner signers[] = new CodeSigner[chainNum];
            signersHardRef = signers;
            for (int j=0; j<chainNum; j++) {
                signers[j] = new CodeSigner(
                        (CertPath)(cpList.get(j)), (Timestamp)null);
            }
	    return signers;

        } catch (CertificateException ce) {
            throw new IOException("Error process signer " + "certificates");
	}
    }

    /*
     * Return a copy of CodeSigner to work around a deserialization
     * hashCode() bug 6799854.
     */
    private CodeSigner newCodeSigner(CodeSigner signer) {
	CertPath cp = signer.getSignerCertPath();
	Timestamp ts = signer.getTimestamp();
	CodeSigner ret = new CodeSigner(cp, newTimestamp(ts));
	return ret;
    }

    /*
     * Return a copy of Timestamp to work around a deserialization
     * hashCode() bug.
     */
    private Timestamp newTimestamp(Timestamp ts) {
	if (ts == null) {
	    return null;
	}
	Date time = ts.getTimestamp();
	CertPath cp = ts.getSignerCertPath();
	return new Timestamp(time, cp);
    }

    public String getNativeLibPath() {
        return getResourceFilename() + "-n";
    }

    //NB: writeFileToDisk() ensure this is run in priviledged block
    private void writeManifest(RandomAccessFile raf, JarFile jar,
            CachedManifest manifest, int contentType, 
            DownloadEngine.DownloadDelegate dd) throws IOException {
        
        URL jarLocation = new URL(url);
	CodeSource[] codeSources = null;
	Object access = null;
	if (enhancedJarAccess) {
	    access = SharedSecrets.javaUtilJarAccess();
	    JavaUtilJarAccess jarAccess = (JavaUtilJarAccess)access;
	    codeSources = jarAccess.getCodeSources(jar, jarLocation);
	}

        ArrayList signerCerts = new ArrayList(); // Certificates
        ArrayList signersCS = new ArrayList(); // CodeSigner
        Certificate[] certs = null;
        CodeSigner[] entrySigners = null;
        Map signerMapCert = new HashMap();
        Map signerMap = new HashMap();
        Map codeSourceCache = new HashMap();
        Map codeSourceCertCache = new HashMap();
	Map signerIndicesCache = new HashMap();
	Map signerIndicesCertCache = new HashMap();
        // Open the output stream
        ByteArrayOutputStream bout = new ByteArrayOutputStream();
        BufferedOutputStream bost = null;
        
        // Jar file authentication
        byte[] buffer = new byte[2048];
        int total = jar.size();
        int count = 0;
        boolean blacklistDone = false;
        boolean trustedLibrariesDone = false;
        int[] singleSignerIndicesCert = null;
        int[] singleSignerIndicesCS = null;
        Enumeration entries = null;

        if (dd != null) {
            dd.validating(jarLocation, 0, total);
        }

	if (!enhancedJarAccess) {
            entries = jar.entries();
	} else {
	    JavaUtilJarAccess jarAccess = (JavaUtilJarAccess)access;
	    if (BlackList.checkJarFile(jar)) {
                throw new JARSigningException(jarLocation, version,
                    JARSigningException.BLACKLISTED);
	    }
	    if (TrustedLibraries.checkJarFile(jar)) {
		Attributes attrs = manifest.getMainAttributes();
		attrs.putValue("Trusted-Library", Boolean.TRUE.toString());
	    }

	    /*
	     * Check for and remove unsigned code source, if any
	     */
	    if (codeSources != null) {
	        List l = new ArrayList();
	        for (int i=0; i < codeSources.length; i++) {
	            if (codeSources[i].getCertificates() != null) {
		        l.add(codeSources[i]);
	            }
	        }
	        if (l.size() != codeSources.length) {
	            codeSources = (CodeSource[])l.toArray(new CodeSource[l.size()]);
	        } else {
	            hasOnlySignedEntries = true;
	        }
	    }
            entries = jarAccess.entryNames(jar, codeSources);

	    if (codeSources != null && codeSources.length == 1) {
                certs = codeSources[0].getCertificates();
                singleSignerIndicesCert = new int[certs.length];
                for (int i = 0; i < certs.length; i++) {
                    signerCerts.add(certs[i]);
                    singleSignerIndicesCert[i] = i;
	        }
                codeSourceCertCache.put(singleSignerIndicesCert, codeSources[0]);

                if (Config.isJavaVersionAtLeast15()) {
                    entrySigners = codeSources[0].getCodeSigners();
                    singleSignerIndicesCS = new int[entrySigners.length];
                    for (int i = 0; i < entrySigners.length; i++) {
                        signersCS.add(entrySigners[i]);
                        singleSignerIndicesCS[i] = i;
	            }
                    codeSourceCache.put(singleSignerIndicesCS, codeSources[0]);
	        }
	        hasSingleCodeSource = true;
	    }
	}
        while (entries.hasMoreElements()) {
            count++;
            String name = null;
            JarEntry entry = null;
	    if (!enhancedJarAccess) {
                entry = (JarEntry)entries.nextElement();
                name = entry.getName();

                if (!blacklistDone) {
                    try {
                        blacklistDone = BlackList.checkJarEntry(jar, entry);
                    } catch (java.security.GeneralSecurityException e) {
                        throw new JARSigningException(new URL(url), version,
                            JARSigningException.BLACKLISTED, e);
                    }
                }
                if (!trustedLibrariesDone) {
                    try {
                        trustedLibrariesDone = TrustedLibraries.checkJarEntry(jar, entry);
                    } catch (java.security.GeneralSecurityException e) {
		        Attributes attrs = manifest.getMainAttributes();
		        attrs.putValue("Trusted-Library", Boolean.TRUE.toString());
                    }
                }
            
                if (isSigningRelated(name) || name.endsWith("/")) {
                    continue;
                }
	    } else {
                name = (String)entries.nextElement();
		entry = jar.getJarEntry(name);
		if (entry == null) {
		    hasMissingSignedEntries = true;
		    Trace.println("signed entry \"" + name + "\" missing from jar " + url, TraceLevel.CACHE);
		}
	    }

            // Authenticate the entry.  To do so, we must read the
            // entire entry through the JarVerifier.VerifierStream
            InputStream in = null;
            int n = 0;
            try {
		if (entry != null) {
                    in = jar.getInputStream(entry);
		}
                if (in != null && DownloadEngine.isNativeContentType(contentType)) {
		    // only extract top level entries of nativelib jars
                    if (name.indexOf("/") == -1 && name.indexOf("\\") == -1) {
                        // Make sure no unicode character in the path 
                        // directory separators
                        File nativeLibPath = new File(
                            getNativeLibPath()).getCanonicalFile();
                        File extractFile = new File(
                            nativeLibPath, name).getCanonicalFile();
                        if (extractFile.getParentFile().equals(nativeLibPath)) {
			    extractFile.getParentFile().mkdirs();
                            bost = new BufferedOutputStream(
                                    new FileOutputStream(extractFile));
                        }
                    }
		}
		if (in != null) {
                    while ((n = in.read(buffer, 0, buffer.length)) != -1) {
                        // Do nothing
                        // Just read. This will throw a security exception if a
                        // signature fails. Native libs are extract at the same 
                        // time
                        if (bost != null) {
			    bost.write(buffer, 0, n);
		        }
                    }
		}
            } catch (SecurityException se) {
                throw new JARSigningException(jarLocation, version,
                        JARSigningException.BAD_SIGNING, se);
            } finally {
                if (bost != null) { bost.close(); bost = null; }
                if (in != null) {
                    in.close();
                }
            }
            
	    if (hasSingleCodeSource) {
                signerMapCert.put(name, singleSignerIndicesCert);
	    } else {
                // Get the signers' certificate obj for this entry
                if (entry != null) {
                    certs = entry.getCertificates();
                } else {
                    JavaUtilJarAccess jarAccess = (JavaUtilJarAccess)access;
                    CodeSource cs = jarAccess.getCodeSource(jar, jarLocation, name);
                    certs = (cs != null) ? cs.getCertificates() : null;
                }
            
                if ((certs != null) && (certs.length > 0)) {
                    int[] signerIndicesCert = new int[certs.length];
                    for (int i = 0; i < certs.length; i++) {
                        // Add the certificate to the list of this Jar file
                        int signerIndexCert = signerCerts.indexOf(certs[i]);
                        if (signerIndexCert == -1){
                            signerIndexCert = signerCerts.size();
                            signerCerts.add(certs[i]);
                        }
                        // Add the certificate to the list of signers for this 
                        // entry
                        signerIndicesCert[i] = signerIndexCert;
                    }
                    String certLine = String.valueOf(signerIndicesCert.length);
                    for (int i = 0; i < signerIndicesCert.length; i++) {
                        certLine += " " + signerIndicesCert[i];
                    }
		    int[] indices = (int[]) signerIndicesCertCache.get(certLine);
                    if (indices == null) {
                        signerIndicesCertCache.put(certLine, signerIndicesCert);
                        CodeSource codeSource = new CodeSource(jarLocation, certs);
                        codeSourceCertCache.put(signerIndicesCert, codeSource);
                    } else {
			signerIndicesCert = indices;
                    }
                    signerMapCert.put(name, signerIndicesCert);
		} // else runtime error
            }
            
            if (Config.isJavaVersionAtLeast15()) {
	        if (hasSingleCodeSource) {
                    signerMap.put(name, singleSignerIndicesCS);
	        } else {
		    if (entry != null) {
		        entrySigners = entry.getCodeSigners();
		    } else {
	    	        JavaUtilJarAccess jarAccess = (JavaUtilJarAccess)access;
		        CodeSource cs = jarAccess.getCodeSource(jar, jarLocation, name);
                        entrySigners = (cs != null) ? cs.getCodeSigners() : null;
		    }
                    if ((entrySigners != null) && (entrySigners.length > 0)) {
                        int[] signerIndicesCS = new int[entrySigners.length];
                        for (int i = 0; i < entrySigners.length; i++) {
                            // Add the entry signer to the list of signers for this
                            // Jar file
                            int signerIndexCS = signersCS.indexOf(entrySigners[i]);
                            if (signerIndexCS == -1){
                                signerIndexCS = signersCS.size();
                                signersCS.add(entrySigners[i]);
                            }
                            // Add the certificate to the list of signers for 
                            // this entry
                            signerIndicesCS[i] = signerIndexCS;
			}
                        String lineCS = String.valueOf(signerIndicesCS.length);
                        for (int i = 0; i < signerIndicesCS.length; i++) {
                            lineCS += " " + signerIndicesCS[i];
                        }
		        int[] indices = (int[]) signerIndicesCache.get(lineCS);
		        if (indices == null) {
			    signerIndicesCache.put(lineCS, signerIndicesCS);
			    CodeSource codeSource = new CodeSource(jarLocation, entrySigners);
			    codeSourceCache.put(signerIndicesCS, codeSource);
		        } else {
			    signerIndicesCS = indices;
		        }
                        signerMap.put(name, signerIndicesCS);
                    } // else { runtime error }
		}
            }
            if (dd != null) {
                if (((count % 10) == 0) || count >= total) {
                    dd.validating(jarLocation, count, total);
                }
            }
        }
        
        // Write the manifest
        if (manifest != null) {
            section3Length = manifest.writeFull(bout);
            reducedManifest2Length = manifest.writeReduced(bout);
            section3Length += reducedManifest2Length;
        } else {
            section3Length = 0;
            reducedManifest2Length = 0;
        }

        ObjectOutputStream out = new ObjectOutputStream(bout);
        BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(out));
        
        if(!signerCerts.isEmpty() ) {
            // Write the list of signers
            out.writeInt(signerCerts.size());
            
            Iterator iterator = signerCerts.iterator();
            while (iterator.hasNext()) {
                out.writeObject(iterator.next());
            }
	    out.flush();
	    if (!enhancedJarAccess) {
	        section4CertsLength = 0;
	    } else {
	        section4CertsLength = bout.size() - section3Length;
	    }

            // Write the map of entry name and signers
            Iterator keys = signerMapCert.keySet().iterator();
            String lastPath = null;
            while (keys.hasNext()) {
                String name = (String)keys.next();
                int[] signerIndicesCert = (int[])signerMapCert.get(name);
                if (name.indexOf("/") != -1) {
                    // Compress path names.  We use a very
                    // rudimentary but effective scheme.  If the
                    // last entry had the same path, we just store
                    // a "/" at the beginning of the entry name.
                    String path =
                            name.substring(0, name.lastIndexOf("/"));
                    if ((lastPath != null) &&
                            path.equals(lastPath)) {
                        name = name.substring(path.length());
                    }
                    lastPath = path;
                }
                
                // Write the entry name
                writer.write(name);
                writer.newLine();
                
                //Write the number of signers, and their names
                String line = String.valueOf(signerIndicesCert.length);
                for (int i = 0; i < signerIndicesCert.length; i++) {
                    line += " " + signerIndicesCert[i];
                }
                writer.write(line, 0, line.length());
                writer.newLine();
            }
            // Write one more line to end certificate
            writer.newLine();
            writer.flush();
	    if (hasStrictSingleSigning()) {
                signerMapCert.clear();
                signerMapCert.put(null, singleSignerIndicesCert);
	    }
        } else {
            //Write list of certificate length as 0
            out.writeInt(0);
	    out.flush();
	    if (!enhancedJarAccess) {
	        section4CertsLength = 0;
	    } else {
	        section4CertsLength = bout.size() - section3Length;
	    }
        }

	if (!enhancedJarAccess) {
	    section4Pre15Length = 0;
	} else {
	    section4Pre15Length = bout.size() - section3Length;
	}
        
        if (!signersCS.isEmpty()) {
            // Write codeSigner obj and list of signers
	    Integer csSize = new Integer(signersCS.size());
	    writer.write(csSize.toString());
            writer.newLine();
            
            //flush the data written by writer
            writer.flush();
            
            Iterator iteratorCS = signersCS.iterator();
            while (iteratorCS.hasNext()) {
                out.writeObject(iteratorCS.next());
            }
	    out.flush();
	    if (!enhancedJarAccess) {
	        section4SignersLength = 0;
	    } else {
	        section4SignersLength = bout.size() - (section3Length + section4Pre15Length);
	    }

            // Write the map of entry name and signersCS
            Iterator keysCS = signerMap.keySet().iterator();
            String lastPathCS = null;
            while (keysCS.hasNext()) {
                String nameCS = (String)keysCS.next();
                int[] signerIndicesCS = (int[])signerMap.get(nameCS);
                if (nameCS.indexOf("/") != -1) {
                    String path = nameCS.substring(0, nameCS.lastIndexOf("/"));
                    if ((lastPathCS != null) &&
                            path.equals(lastPathCS)) {
                        nameCS = nameCS.substring(path.length());
                    }
                    lastPathCS = path;
                }
                
                writer.write(nameCS);
                writer.newLine();
                
                // Write the number of signers, and their names
                String lineCS = String.valueOf(signerIndicesCS.length);
                for (int i = 0; i < signerIndicesCS.length; i++) {
                    lineCS += " " + signerIndicesCS[i];
                }
                writer.write(lineCS, 0, lineCS.length());
                writer.newLine();
            }
	    if (hasStrictSingleSigning()) {
                signerMap.clear();
                signerMap.put(null, singleSignerIndicesCS);
	    }
        } else {
            //Write list of signers length as 0
            writer.write("0");

	    /*
	     * Latent bug fixed in 6u19. Missing newLine() causes readLine() to
	     * return null when reading CodeSigners but this is/was covered up by
	     * defaulting numCS to 0 when readLine() returns null.
	     */
            writer.newLine();
	    out.flush();
	    if (!enhancedJarAccess) {
	        section4SignersLength = 0;
	    } else {
	        section4SignersLength = bout.size() - (section3Length + section4Pre15Length);
	    }
        }
        
        // flush the data written by writer
        writer.flush();

        //Flush the outer stream to take care of caching
        //jar files not having manifest
        out.flush();
        raf.write(bout.toByteArray());
        section4Length = bout.size() - section3Length;

        if (Config.isJavaVersionAtLeast15()) {
            if (!signersCS.isEmpty()) {
                CodeSigner signers[] = new CodeSigner[signersCS.size()];
                signersHardRef = (CodeSigner[]) signersCS.toArray(signers);
                signersRef = new SoftReference(signersHardRef);
                signerMapHardRef = signerMap;
                signerMapRef = new SoftReference(signerMapHardRef);
                codeSourceCacheHardRef = codeSourceCache;
                codeSourceCacheRef = new SoftReference(codeSourceCacheHardRef);
            } else {
                signersHardRef = null;
                signersRef = null;
                signerMapHardRef = null;
                signerMapRef = null;
                codeSourceCacheHardRef = null;
                codeSourceCacheRef = null;
            }
        } else {
            if (!signerCerts.isEmpty()) {
                Certificate certificates[] = 
                                     new Certificate[signerCerts.size()];
                certificatesHardRef = (Certificate[])
                                     signerCerts.toArray(certificates);
                certificatesRef = new SoftReference(certificatesHardRef);
                signerMapCertHardRef = signerMapCert;
                signerMapCertRef = new SoftReference(signerMapCertHardRef);
                codeSourceCertCacheHardRef = codeSourceCertCache;
                codeSourceCertCacheRef = new SoftReference(codeSourceCertCacheHardRef);
            } else {
                certificatesHardRef = null;
                certificatesRef = null;
                signerMapCertHardRef = null;
                signerMapCertRef = null;
                codeSourceCertCacheHardRef = null;
                codeSourceCertCacheRef = null;
            }
        }
    }

    /* Used to support in place upgrade from cache version 602.
     * Will be removed in the future */
    private void readHeadersOld(RandomAccessFile raf) throws IOException{
        final String CODEBASEIP_HEADER_KEY = "deploy_resource_codebase_ip";

        //Read the header fields as name-value paris
        try {
            for(int count = raf.readInt();count > 0; count--) {
                String name = raf.readUTF();
                if (name.equals(CODEBASEIP_HEADER_KEY)) {
                    // extract codebase ip from header
                    setCodebaseIP(raf.readUTF());
                    // codebase ip is not really part of the real http headers
                    // so don't add it to headerFields'
                } else {
                    if(name.equals("<null>"))
                        name = null;
                    headerFields.add(name, raf.readUTF());
                }
            }
        } finally {
            //do not close the stream since raf could be used
            //by the calling method
        }
    }
    
    private void readHeaders(DataInputStream is) throws IOException{
        //Read the header fields as name-value paris
        try {
            for(int count = is.readInt(); count > 0; count--) {
                String name = is.readUTF();
                if(name.equals("<null>"))
                        name = null;
                headerFields.add(name, is.readUTF());
            }
        } finally {
            //do not close the stream since it could be used
            //by the calling method
        }
    }
    
    //Writes the header into the cache index file
    private void writeHeaders(DataOutputStream out) throws IOException {
        try {
            if (headerFields == null) {
                out.writeInt(0);
                return;
            }

            // To get size
            Map headers = headerFields.getHeaders();
            if(!headers.isEmpty()) {
                // Write the list of signers
                out.writeInt(headers.size());
               // Write all headers, still keep in some kind of order
                for(int index = 0; index < headers.size(); index ++) {
                    String name = headerFields.getKey(index);
                    if (null == name) {
                        name = "<null>";
                    }
                    out.writeUTF(name);
                    out.writeUTF(headerFields.getValue(index));
                }
               
            } else {
                //Write list of header fields as 0
                out.writeInt(0);
            }      
        } finally {
            //do not close the stream since raf could be used
            //by the calling method
        }
    }
    
    private String printManifest() {
        Manifest manifest = getManifest();

        if (manifest != null) {
            StringBuffer sb = new StringBuffer();
            Attributes a = manifest.getMainAttributes();
            Iterator i = a.keySet().iterator();
            while (i.hasNext()) {
                Object key = i.next();
                sb.append("key: " + key);
                sb.append(" value: " + a.get(key) + "\n");
            }
            return sb.toString();
        }
        return null;
    }
    
    public String toString() {
        StringBuffer sb = new StringBuffer();
        sb.append("-----Cache Entry------\n");
        sb.append("busy: " + getBusy() + "\n");
        sb.append("incomplete: " + getIncomplete() + "\n");
        sb.append("cacheVersion: " + getCacheVersion() + "\n");
        sb.append("forceUpdate: " + getForceUpdate() + "\n");
        sb.append("noHref: " + getNoHref() + "\n");
        sb.append("contentLength: " + getContentLength() + "\n");
        long lastModified = getLastModified();
        sb.append("lastModified: " + lastModified + " [" + 
                new Date(lastModified).toString() + "]\n");
        sb.append("expirationDate: " + getExpirationDate() + "\n");
        sb.append("version: " + getVersion() + "\n");
        sb.append("URL: " + url + "\n");
        sb.append("NamespaceID: " + getNamespaceID() + "\n");
        sb.append("HTTP/HTTPS Header: " + getHeaders() + "\n");
        if (getManifest() != null) {
            sb.append("Jar-Manifest Main Attributes:\n");
            sb.append(printManifest());
            sb.append("----------------------\n");
        }
        return sb.toString();
    }

    static CodeSource getUnsignedCS(URL url) {
	return new CodeSource(url, (Certificate[]) null);
    }

    // true if file is part of the signature mechanism itself
    public static boolean isSigningRelated(String name) {
        name = name.toUpperCase(Locale.ENGLISH);
	if (!name.startsWith("META-INF/")) {
	    return false;
	}
	name = name.substring(9);
	if (name.indexOf('/') != -1) {
	    return false;
	}
        if (name.endsWith(".DSA") ||
            name.endsWith(".RSA") ||
            name.endsWith(".SF")  ||
            name.endsWith(".EC")  ||
	    name.startsWith("SIG-") ||
	    name.equals("MANIFEST.MF")) {
	    return true;
	}
	return false;
    }
}

class IndexFileObjectInputStream extends ObjectInputStream {

    public IndexFileObjectInputStream(InputStream in) throws IOException {
        super(in);
    }

    // This is to ensure all ClassLoading triggered by the
    // de-serialization of signer/cert classes stored in the cache index
    // file is handled by the SystemClassLoader. (instead of
    // plugin/webstart ClassLoader)
    //
    // We assume the only thing being deserialized is an array of
    // X509 certificates but these are actually converted into
    // java.security.cert.Certificate.CertificateRep instances which
    // makes them provider-independent. So, all classes needing to be
    // resolved will inherently just be core classes from the
    // bootstrap loader.
    //
    // this is to prevent deadlock case as seen by 6949705
    protected Class resolveClass(ObjectStreamClass desc)
            throws IOException, ClassNotFoundException {
        String name = desc.getName();

        return Class.forName(name, false,
                ClassLoader.getSystemClassLoader());

    }
}

