/*
 * @(#)SigningInfo.java	1.54 10/04/26
 * 
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.security;
import java.io.InputStream;
import java.io.IOException;
import java.net.URL;
import java.util.*;
import java.util.jar.JarFile;
import java.util.jar.JarEntry;
import java.security.cert.Certificate;
import com.sun.javaws.Globals;
import com.sun.deploy.net.JARSigningException;
import com.sun.deploy.cache.Cache;
import com.sun.deploy.cache.CacheEntry;
import com.sun.deploy.cache.DeployCacheJarAccess;
import com.sun.deploy.cache.DeployCacheJarAccessImpl;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.config.Config;
import java.security.CodeSigner;
import java.security.CodeSource;
import java.security.cert.X509Certificate;
import java.security.cert.CertPath;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

/*
 * This class performs validation for single resource.
 * It covers both cached and non-cached resources as well as
 * uses different approaches to validate them depending on version of
 * JRE (pre5 or 5+).
 *
 * It also provide access to cached information about results of previous
 * validation (and means to update it).
 *
 * For JAR files following assertions are validated:
 *   - all entries are signed
 *   - there is a set of common certificate chains applicable to all entries
 */

public class SigningInfo {

    /* Helper class to work with set of certificates
       as we set of certificate chains */
    static class CertChain {
        Certificate[] certs;

        CertChain(Certificate[] certs, int startIndex, int endIndex) {
            this.certs = new Certificate[endIndex - startIndex + 1];
            for (int j = 0; j < this.certs.length; j++) {
                this.certs[j] = certs[startIndex + j];
            }
        }

        Certificate[] getCertificates() {
            return certs;
        }

        int getLength() {
            return certs.length;
        }

        public int hashCode() {
            if (certs.length == 0) {
                return 0;
            }
            return certs[0].hashCode();
        }

        public boolean equals(Object o) {
            CertChain c = (CertChain) o;

            if (c == null || c.getLength() != getLength()) {
                return false;
            }

            for (int i = 0; i < certs.length; i++) {
                if (!c.certs[i].equals(certs[i])) {
                    return false;
                }
            }
            return true;
        }
    }

    static private DeployCacheJarAccess jarAccess =
	DeployCacheJarAccessImpl.getJarAccess();


    /* Helper method to export list of CertChain objects as array 
       Will return null if list consists of objects of other type */
    public static Certificate[] toCertificateArray(List chains) {
        //count total number of certificates
        Iterator it = chains.iterator();
        int len = 0;
        while(it.hasNext()) {
            Object o = it.next();
            if (o instanceof CertChain) {
                len += ((CertChain) o).getLength();
            } else {
                return null; //only support CertChain sets here!
            }
        }
        //allocate and fill result array
        Certificate ret[] = new Certificate[len];
        it = chains.iterator();
        int i = 0;
        while(it.hasNext()) {
            Certificate[] chain = ((CertChain) it.next()).getCertificates();
            System.arraycopy(chain, 0, ret, i, chain.length);
            i += chain.length;
        }
        return ret;
    }

    /* Workaround for bug 6799854
     * We can not rely on CodeSigner.hashCode() */
    private static boolean setContains(List s, Object o) {
        if (s == null || o == null) return false;

        Iterator it = s.iterator();

	// CodeSigner class only available in JRE 1.5 and later
	if (Config.isJavaVersionAtLeast15()) {
	    CertPath cp = null;
            CertPath oCertPath = ((CodeSigner)o).getSignerCertPath();
            while(it.hasNext()) {
                // only compare CertPath in CodeSigner, not Timestamp object
                cp = ((CodeSigner)(it.next())).getSignerCertPath();
                if (oCertPath.equals(cp))
                    return true;
            }
            return false;
	}
	else { // JRE 1.4.2 and early
 	    while(it.hasNext()) {
 		if (o.equals(it.next()))
                    return true;
            }
            return false;
	}
    }

    /* helper methods to deal with operations on lists of certificate chains */

    public static List overlapChainLists(List oldCertChains, List jarCertChains) {
        if (oldCertChains == null || jarCertChains == null)
            return null;

        List next = new ArrayList();

        Iterator it = oldCertChains.iterator();
        while (it.hasNext()) {
           Object o = it.next();
           //use workaround as it could be sets of CodeSigner
           if (setContains(jarCertChains, o)) {
               next.add(o);
           }
        }

        if (next.isEmpty())
            return null;

        return next;
    }

    public static List /*<CodeSigner>*/ overlapSigners(
            List/* <CodeSigner> */ commonSigners, CodeSigner signers[]) {
        List /*<CodeSigner>*/ next = new ArrayList/* <CodeSigner> */();

        for(int i=0; i<signers.length; i++) {
            //first entry, take its signers as common
            if (commonSigners == null || setContains(commonSigners,signers[i])) {
                next.add(signers[i]);
            }
        }
        return next;
    }

    public static List/* <CertChain> */ overlapCertificateChains(
            List/* <CertChain> */ commonSigners, Certificate certs[]) {
        List/* <CertChain> */ next = new ArrayList/* <CodeSigner> */();
        int startIndex = 0;
        CertChain certChain;
        while ((certChain = getAChain(certs, startIndex)) != null) {
            if (commonSigners == null || commonSigners.contains(certChain)) {
                next.add(certChain);
            }
            startIndex += certChain.getLength();
        }
        return next;
    }

    /**
     * Helper method that extracts ONE certificate chain from the specified
     * certificate array which may contain multiple certificate chains,
     * starting from index 'startIndex'.
     */
    private static CertChain getAChain(Certificate[] certs,
            int startIndex) {
        if (startIndex > certs.length - 1) {
            return null;
        }

        int i = 0;
        // Keep going until the next certificate is not the
        // issuer of this certificate.
        for (i = startIndex; i < certs.length - 1; i++) {
            if (!((X509Certificate) certs[i + 1]).getSubjectDN().
                    equals(((X509Certificate) certs[i]).getIssuerDN())) {
                break;
            }
        }

        return new CertChain(certs, startIndex, i);
    }

    private CacheEntry ce = null;
    private URL location = null;
    private String version = null;
    private boolean canBeSkipped;
    private String jarFilePath = null;

    public SigningInfo(URL location, String version) {
        this.location = location;
        this.version = version;

        boolean ignoreVersion = false;
        try {
            jarFilePath = DownloadEngine.getCachedResourceFilePath(location, version);
            canBeSkipped = false;
        } catch (IOException ioe) {
            // resource not cached
            // for example lazy jars will not be downloaded yet during
            // application first launch
            if (version != null) {
                //workaround for case when xml file has version but
                //http response header had no version and cacheentry version is null
                //NB: we should use actual version used to find cache
                //    entry in the cache (revise creation of LauncgDesc?)
                try {
                    jarFilePath = DownloadEngine.getCachedResourceFilePath(location, null);
                    ignoreVersion = true;
                } catch (IOException e) {
                    canBeSkipped = true;
                }
            } else {
                canBeSkipped = true;
            }
        }

        if (!canBeSkipped && Cache.isCacheEnabled()) {
           // cache enabled case
           // get jarfile from cache
           ce = Cache.getCacheEntry(location, null, ignoreVersion ? null : version);
        }
    }

    private boolean wasChecked = false;

    /* Perform actual validation of cached or non-cached jars.
     * For cached jars also updates cache entry with latest validation status
     */
    public List /*<CodeSigner> or <Certificate[]>*/ check()
            throws IOException, JARSigningException {
        List result = null;
        if (ce != null) {
                JarFile f = ce.getJarFile();
                Trace.println("Validating cached jar url=" + ce.getURL() +
                  " ffile=" + ce.getResourceFilename() + " " + f,
                              TraceLevel.SECURITY);
                result = getCommonCodeSignersForJar(f);
                if (result != null && result.isEmpty()) {
                    result = null;
                    throw new JARSigningException(location, version,
                            JARSigningException.MULTIPLE_SIGNERS);
                }
               //we can not cache result of validation here as 
               // certificate might be accepted only temporary or even rejected
               //This is espicially important for case of applets where 
               // there is no requirement that all jars are signed by same certificate!
               wasChecked = true;
        } else {
                JarFile jarFile = null;
                try {
                    // verify the jar file in the temp location
                    jarFile = new JarFile(jarFilePath);
                    result = getCommonCodeSignersForJar(jarFile);
                    if (result != null && result.isEmpty()) {
                        result = null;
                        throw new JARSigningException(location, version,
                                JARSigningException.MULTIPLE_SIGNERS);
                    }
                } finally {
                    if (jarFile != null) {
                        jarFile.close();
                    }
                }
        }
        return result;
    }

    public long getCachedVerificationTimestampt() {
        if (ce != null)
          return ce.getValidationTimestampt();
        return 0;
    }

    public List /*<CodeSigner> or <CertChain[]>*/ getCertificates() {
        if (ce != null) {
            if (Globals.isJavaVersionAtLeast15()) {
                return overlapSigners(null, ce.getCodeSigners());
            } else {
                return overlapCertificateChains(null, ce.getCertificates());
            }
        }
        return null;
    }

    //lazyly loaded jars covered here
    public boolean canBeSkipped() {
        return canBeSkipped;
    }

    //use cached result of validation unless it is not available
    public boolean isKnownToBeValidated() {
        return (ce != null && ce.getValidationTimestampt() != 0);
    }

    public boolean isKnownToBeSigned() {
        if (ce != null)
          return ce.isKnownToBeSigned();
        return false;
    }

    /* List of resources and their timestampts used for
     * previous validation cycle.
     * We can only reuse validation result if set of resources is
     * exactly the same as it was at the time of validation */
    public Map getTrustedEntries() {
        if (ce != null) {
            return ce.getCachedTrustedEntries();
        }
        return null;
    }

    /* update only if we are not using cached results already. */
    public void updateCacheIfNeeded(boolean result,
           Map /*<String, Long> */ trustedEntries,
           long timestampt, long expiration) {

       if (!wasChecked) return;
       updateCache(result, trustedEntries, timestampt, expiration);
    }

    public void updateCache(boolean result, 
            Map /*<String, Long> */ trustedEntries, 
            long timestampt, long expiration) {
        if (ce != null) {
          ce.updateValidationResults(result, trustedEntries, timestampt, expiration);
        }
    }

    /* Scans all entries in the jar files and return list of certificate chains
     * that is common for all entries (with few exceptions such as META-INF).
     * We need to use List instead of Set because we will always parse the latest
     * certificate chain first.
     *
     * Throws exception if anything is wrong (including empty result list) */
    List /*<CodeSigner> or <Certificate[]>*/ getCommonCodeSignersForJar(JarFile jf) throws IOException {
        List/* <CodeSigner> */ commonSigners = null;

	boolean jarEntryFound = false;

        try {
            Enumeration entries;
            boolean isJDK5orNewer = Globals.isJavaVersionAtLeast15();
	    if (jarAccess != null) {
		// j2se bug workaround
		jarAccess.getCodeSource(jf, new URL("http:"), "/NOP");
	        CodeSource[] codeSources = jarAccess.getCodeSources(jf, null);
	        jarAccess.setEagerValidation(jf, true);
	        entries = jarAccess.entryNames(jf, codeSources);

                //stop at the end or when set of common signers is empty
		//someday... optimize cached jars by using CacheEntry.hasStrictSingleSigner()
                while (entries.hasMoreElements() &&
                    (commonSigners == null || !commonSigners.isEmpty())) {
                    String entryName = (String) entries.nextElement();
		    CodeSource cs = jarAccess.getCodeSource(jf, null, entryName);
		    jarEntryFound = true;
                    if (isJDK5orNewer) {
                        CodeSigner signers[] = cs.getCodeSigners();
                        if (signers == null) {
                            // Ignore these for backwards compatibility with pre-1.5 JarSigner
                            if (entryName.startsWith("META-INF/")) {
                        	continue;
			    }

                            Trace.println("Found unsigned entry: " + entryName,
                                TraceLevel.SECURITY);
                            throw new JARSigningException(location, version,
                                          JARSigningException.UNSIGNED_FILE);
                        } else {
                            commonSigners = overlapSigners(commonSigners, signers);
                        }
                    } else {
                        Certificate[] certs = cs.getCertificates();
                        if (certs == null) {
                            // Ignore these for backwards compatibility with pre-1.5 JarSigner
                            if (entryName.startsWith("META-INF/")) {
                        	continue;
			    }

                            Trace.println("Found unsigned entry: " + entryName,
                                TraceLevel.SECURITY);
                            throw new JARSigningException(location, version,
                                          JARSigningException.UNSIGNED_FILE);
                        } else {
                            commonSigners = overlapCertificateChains(
                                commonSigners, certs);
			}
                    }
                }
            } else {
	        entries = jf.entries();
                //stop at the end or when set of common signers is empty
                while (entries.hasMoreElements() &&
                    (commonSigners == null || !commonSigners.isEmpty())) {
                    byte[] buffer = new byte[8192];
                    JarEntry je = (JarEntry) entries.nextElement();
                    String entryName = je.getName();
		    if (!CacheEntry.isSigningRelated(entryName) && 
                            !entryName.endsWith("/")) {

			jarEntryFound = true;
                        InputStream is = jf.getInputStream(je);

                        // Read in each jar entry. A security exception will
                        // be thrown if a signature/digest check fails.
                        while (is.read(buffer, 0, buffer.length) != -1) {
                        }

                        is.close();

                        if (isJDK5orNewer) {
                            CodeSigner signers[] = je.getCodeSigners();
                            if (signers == null) {
                        	// Ignore these for backwards compatibility with pre-1.5 JarSigner
                        	if (entryName.startsWith("META-INF/")) {
                        	    continue;
				}

                                Trace.println("Found unsigned entry: " + entryName,
                                    TraceLevel.SECURITY);
                                throw new JARSigningException(location, version,
                                              JARSigningException.UNSIGNED_FILE);
                            } else {
                                commonSigners = overlapSigners(commonSigners, signers);
                            }
                        } else {
                            Certificate[] certs = je.getCertificates();
                            if (certs == null) {
                        	// Ignore these for backwards compatibility with pre-1.5 JarSigner
                        	if (entryName.startsWith("META-INF/")) {
                        	    continue;
				}

                                Trace.println("Found unsigned entry: " + entryName,
                                    TraceLevel.SECURITY);
                                throw new JARSigningException(location, version,
                                              JARSigningException.UNSIGNED_FILE);
                            } else {
                                commonSigners = overlapCertificateChains(
                                    commonSigners, certs);
                            }
                        }
                    }
                }
            }
        } catch (JARSigningException jse) {
            throw jse;
        } catch (IOException ioe) {
            throw new JARSigningException(location, version,
                    JARSigningException.BAD_SIGNING, ioe);
        } catch (SecurityException e) {
            throw new JARSigningException(location, version,
                    JARSigningException.BAD_SIGNING, e);
        }

	// If no entry inside jar file, we will skip the certificate check
	canBeSkipped = !jarEntryFound;

        return commonSigners;
    }

}

