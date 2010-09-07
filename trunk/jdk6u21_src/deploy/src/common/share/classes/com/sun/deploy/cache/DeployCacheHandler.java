/*
 * @(#)DeployCacheHandler.java	1.63 10/03/24
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.cache;

import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.OutputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.BufferedInputStream;
import java.io.PrintStream;
import java.io.InputStream;
import java.io.File;
import java.net.CacheRequest;
import java.net.CacheResponse;
import java.net.ResponseCache;
import java.net.URI;
import java.net.URL;
import java.net.URLConnection;
import java.net.HttpURLConnection;
import java.util.HashSet;
import java.util.Map;
import java.util.List;
import java.util.Iterator;
import java.security.AccessController;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.Principal;
import javax.net.ssl.SSLPeerUnverifiedException;
import com.sun.deploy.net.DownloadEngine;
import com.sun.deploy.net.CanceledDownloadException;
import com.sun.deploy.net.HttpUtils;
import com.sun.deploy.util.Trace;
import com.sun.deploy.config.Config;
import sun.awt.AppContext;
import com.sun.deploy.Environment;
import com.sun.deploy.net.offline.DeployOfflineManager;
import com.sun.deploy.net.HttpRequest;
import java.io.BufferedOutputStream;
import java.util.HashMap;

/**
 * A callback mechanism to be set up with URLConnection to enable
 * access to caching management.
 *
 * A CacheHandler can be registered with URLConnection by doing a
 * URLConnection.setDefaultCacheHandler(CacheHandler). The currently
 * registered CacheHandler is stored in a protected field in
 * URLConnection.
 *
 * @since 1.5
 */
public class DeployCacheHandler extends java.net.ResponseCache {

    //global set of URL for which we know we do not cache them
    //NB: why we need it?
    //    obviously to reduce cost of subsequent checks for the same URL
    //    However, how often this happens and how much we save?
    //    One typical scenario is poor programming style in JavaFX
    //    where developer may create many copies of the same image (with same URL).
    //    It seems that check is relatively not expensive though and
    //    memory is never released, so may be we should just drop it later.
    // Access to it MUST be protected by synchronized blocks
    private final static HashSet isNotCacheable = new HashSet();

    //This is map of URLs to lock objects used to avoid performing double update checks
    //Access to it MUST be protected by synchronized blocks
    private final HashMap inProgress = new HashMap();

    //storage used to mark URLs for which pack200 should be used
    //It is set in the PluginURLCachedJarCallback and the assumption here
    // is that cache handler is called on the same thread
    private final static ThreadLocal isDeployPackURL = new ThreadLocal();

    //storage used to prevent recurssion due to attempt to perform update check
    private ThreadLocal inCacheHandler = new ThreadLocal();

    public static void setDeployPackURL(URL u) {
        isDeployPackURL.set(u);
    }

    public static void clearDeployPackURL() {
        isDeployPackURL.set(null);
    }

    public static void reset() {
        // Set system wide cache handler
        ResponseCache.setDefault(new DeployCacheHandler());
        clearDeployPackURL();
    }

    /**
     * Retrieve the cached response based on the requesting uri,
     * request method and request headers. Typically this method is
     * called by the protocol handler before it sends out the request
     * to get the network resource. If a cached response is returned,
     * that resource is used instead.
     *
     * @param uri a <code>URI</code> used to reference the requested
     *            network resource
     * @param rqstMethod a <code>String</code> representing the request
     *            method
     * @param rqstHeaders - a Map from request header
     *            field names to lists of field values representing
     *            the current request headers
     * @returns a <code>CacheResponse</code> instance if available
     *          from cache, or null otherwise
     * @throws	IOException if an I/O error occurs
     * @throws  IllegalArgumentException if any one of the arguments is null
     *
     * @see     java.net.URLConnection#setUseCaches(boolean)
     * @see     java.net.URLConnection#getUseCaches()
     * @see     java.net.URLConnection#setDefaultUseCaches(boolean)
     * @see java.net.URLConnection#getDefaultUseCaches()
     */
    public CacheResponse get(final URI uri, String rqstMethod,
            Map requestHeaders) throws IOException {
        /* Implementation should obey following assertions:
         *   - prevent concurrent update checks for the same resource
         *   - avoid recursion due to attempt to perform update check inside this method
         *   - do not block attempts to work with different resources
         */

        CacheResponse response = null;
        if (!Cache.isCacheEnabled() || !rqstMethod.equals("GET")
                // Network update check may caused recursion -
                //   if we already have cache handler in stack trace
                //   then return immediately to avoid unneeded recursion
                || inCacheHandler.get() != null) {
            return null;
        }

        Object key;
        //make sure there is lock object for given URI
        synchronized (inProgress) {
            if (!inProgress.containsKey(uri)) {
                inProgress.put(uri, new Object());
            }
            key = inProgress.get(uri);
        }

        //only one thread can be checking for presence of this resource at time
        synchronized (key) {
            try {
                inCacheHandler.set(Boolean.TRUE);
                File cachedFile = null;
                boolean isPackEnabled = false;
                URL origUrl = (URL) isDeployPackURL.get();
                if (origUrl != null) {
                    isPackEnabled = true;
                }
                // use orignal url for cache lookup if pack200 is enabled
                final URL url = isPackEnabled ? origUrl : uri.toURL();

                final URL urlNoQuery = HttpUtils.removeQueryStringFromURL(url);

                final String jarVersion = (String) AppContext.getAppContext().get(
                        Config.getAppContextKeyPrefix() + urlNoQuery);

                // try get cached resource if no updates
                // we need to use the original url (with query) for update check
                boolean updateAvail = false;
                if (!DeployOfflineManager.isGlobalOffline()) {
                    // don't do update check if offline
                    // use cache resources directly

                    //NB: download engine will not perform update check twice
                    //    However, it may issue two concurrent update checks
                    //    and we need to protect against this
                    //    (to large extent this is why we synchronize on key
                    //      above but it also help to avoid double work reading
                    //      same entry from the cache (once read it will be
                    //      served from memory cache))
                    updateAvail = DownloadEngine.isUpdateAvailable(
                            url, jarVersion, isPackEnabled, requestHeaders);
                }

                if (updateAvail == false) {
                    try {
                        cachedFile = (File) AccessController.doPrivileged(
                                new PrivilegedExceptionAction() {
                                    public Object run() throws IOException {
                                        // the resource requested can be any resource type

                                        CacheEntry ce = Cache.getCacheEntry(
                                                jarVersion == null ? url : urlNoQuery,
                                                null, jarVersion);
                                        if (ce != null) {
                                            return new File(ce.getResourceFilename());
                                        }
                                        return null;
                                    }
                                });
                    } catch (PrivilegedActionException pae) {
                        //do not do anything
                        //will consider it as no resource in cache
                        Trace.ignoredException(pae);
                    }
                }

                if (cachedFile == null) {
                    return null;
                }

                InputStream is = null;
                final File cachedFileF = cachedFile;
                if (cachedFileF != null) {
                    try {
                        is = (InputStream) AccessController.doPrivileged(
                                new PrivilegedExceptionAction() {
                                    public Object run() throws IOException {
                                        return new FileInputStream(cachedFileF);
                                    }
                                });
                    } catch (PrivilegedActionException e) {
                        Trace.ignoredException(e);
                    }

                    if (is != null) {
			// get the header values from cache
                        Map cachedHeadersNEW = DownloadEngine.getCachedHeaders(
                                jarVersion == null ? url : urlNoQuery, null, jarVersion, null, false);

                        if (uri.getScheme().equals("https")) {
                            response = new DeploySecureCacheResponse(is,
                                    cachedHeadersNEW);
                        } else {
                            response = new DeployCacheResponse(is,
                                    cachedHeadersNEW);
                        }
                    }
                }
            } finally {
                inCacheHandler.set(null);

                //remove object we used for synchronization
                synchronized (inProgress) {
                    inProgress.remove(key);
                }
            }
        }

        return response;
    }

    private static boolean isResourceCacheable(String url, URLConnection conn) {
        // do not cache resource if:
        // 1. cache disabled
        // 2. useCaches is set to false and resource is non jar/zip file
        // 3. connection is not a GET request
        // 4. cache-control header is set to no-store
        // 5. lastModified and expiration not set
        // 6. resource is a partial body resource
        if (!Cache.isCacheEnabled() ||
                (conn.getUseCaches() == false &&
                DownloadEngine.isAlwaysCached(url) == false)) {
            return false;
        }
        if (conn instanceof HttpURLConnection) {
            // only support http GET request
            if (!(((HttpURLConnection)conn).getRequestMethod().equals("GET"))) {
                return false;
            }
        }

        if (conn.getHeaderField("content-range") != null) {
            // do not cache if it's a partial body resource
            return false;
        }

        String cacheControlHeader = conn.getHeaderField("cache-control");
        if (cacheControlHeader != null &&
                cacheControlHeader.toLowerCase().indexOf("no-store") != -1) {
            return false;
        }
        if (conn.getLastModified() == 0 && conn.getExpiration() == 0) {
            // this resource will never be cached
            synchronized (isNotCacheable) {
                isNotCacheable.add(url);
            }
            return false;
        }

        return true;
    }

    // test to see if the URL in question is in our "not cached" HashSet
    public static boolean resourceNotCached(String url) {
        synchronized (isNotCacheable) {
            return isNotCacheable.contains(url);
        }
    }

    /**
     * The protocol handler calls this method after a resource has
     * been retrieved, and the ResponseCache must decide whether or
     * not to store the resource in its cache. If the resource is to
     * be cached, then put() must return a CacheRequest object which
     * contains a WriteableByteChannel that the protocol handler will
     * use to write the resource into the cache. If the resource is
     * not to be cached, then put must return null.
     *
     * @param uri a <code>URI</code> used to reference the requested
     *            network resource
     * @param conn - a URLConnection instance that is used to fetch
     *            the response to be cached
     * @returns a <code>CacheRequest</code> for recording the
     *            response to be cached. Null return indicates that
     *            the caller does not intend to cache the response.
     * @throws IOException if an I/O error occurs
     * @throws IllegalArgumentException if any one of the arguments is
     *            null
     */
    public CacheRequest put(URI uri, URLConnection conn)
            throws IOException {
        if (isResourceCacheable(uri.toString(), conn) == false) {
            return null;
        }
        URL origUrl = (URL) isDeployPackURL.get();
        boolean isPackEnabled = false;
        if (origUrl != null) {
            isPackEnabled = true;
        }
        // use orig url for cache if pack200 enabled
        URL url = isPackEnabled ? origUrl : uri.toURL();

        return new DeployCacheRequest(url, conn, isPackEnabled);
    }
}

class DeployFileOutputStream extends FileOutputStream {

    private URL _url;
    private URLConnection _conn;
    private File _file;
    private boolean _isPack = false;
    private boolean closed = false;
    private boolean finalized = false;
    private boolean aborted = false;

    DeployFileOutputStream(File file, URL url, URLConnection conn,
            boolean isPack) throws FileNotFoundException {
        super(file);
        _url = url;
        _conn = conn;
        _file = file;
        _isPack = isPack;
    }

    void setAbort(boolean b) {
        aborted = b;
    }

    protected void finalize() throws IOException {
        finalized = true;
        super.finalize();
    }

    public void close() throws IOException {
        super.close();

        if (closed || finalized || aborted) {
            // no need to cache the same url multiple times
            // don't store into cache if it's finalized
            // don't store into cache if the cache request is aborted
            return;
        }

        closed = true;

        URL urlNoQuery = HttpUtils.removeQueryStringFromURL(_url);
        String query = _url.getQuery();

        String jarVersion = (String)AppContext.getAppContext().get(
                Config.getAppContextKeyPrefix() + urlNoQuery.toString());
        boolean applyJarDiff = false;

        String mimeType = _conn.getContentType();
        if (mimeType != null &&
                mimeType.equalsIgnoreCase("application/x-java-jnlp-error")) {
            throw new IOException("version requested not returned");
        }

        String downloadVersion = _conn.getHeaderField("x-java-jnlp-version-id");
        if (downloadVersion == null && Environment.isJavaPlugin()) {
            // old style plugin jar versioning support
            downloadVersion = jarVersion;
        }


        applyJarDiff = (mimeType != null) &&
                mimeType.equalsIgnoreCase("application/x-java-archive-diff");

        InputStream fis = null;

        try {
            fis = (InputStream) AccessController.doPrivileged(
                    new PrivilegedExceptionAction() {
                        public Object run() throws IOException {
                            return new BufferedInputStream(new FileInputStream(_file));
                        }
                    });
        } catch (PrivilegedActionException e) {
            Trace.ignoredException(e);
        }

        // Get content type
        String contentType = _conn.getRequestProperty(HttpRequest.CONTENT_TYPE);

        try {
            int cType = 0;

            if (_isPack) {
                cType = DownloadEngine.PACK200_CONTENT_BIT | DownloadEngine.JAR_CONTENT_BIT;
            } else {
                if (contentType != null && contentType.equals(HttpRequest.JAR_MIME_TYPE)) {
                    cType = DownloadEngine.JAR_CONTENT_BIT;
                } else {
                    cType = DownloadEngine.NORMAL_CONTENT_BIT;
                }
            }

            CacheEntry ce = Cache.downloadResourceToCache(urlNoQuery,
                    downloadVersion, _conn, _url, applyJarDiff, cType, fis);

            if (ce != null) {
                CacheEntry oldCE = (CacheEntry)(MemoryCache.addLoadedResource(
                        urlNoQuery.toString(), ce));

                // if there is a previous loaded entry of the same url,
                // mark it as incomplete and let clean-up thread take care of it
                // should not remove old versioned resource
                // Only non-versioned resource should be mark incomplete
                // if they are out dated.   So both ce and oldCE should
                // not be versioned resources.
                if (oldCE != null && oldCE.getVersion() == null &&
                        ce.getVersion() == null) {
                    Cache.markResourceIncomplete(oldCE);
                }
            }
        } catch (CanceledDownloadException ce) {
            throw new IOException(ce.getMessage());
        } finally {
            super.close();
            try {
                AccessController.doPrivileged(
                        new PrivilegedExceptionAction() {
                            public Object run() throws IOException {

                                if (_file != null) {
                                    _file.delete();
                                }
                                return null;
                            }
                        });
            } catch (PrivilegedActionException e) {
                Trace.ignoredException(e);
            }
        }
    }
}

class DeployCacheRequest extends java.net.CacheRequest {
    DeployFileOutputStream fos;
    File file;
    public DeployCacheRequest(final URL url, final URLConnection conn,
            final boolean isPack) {
        try {
            java.security.AccessController.doPrivileged(
                    new java.security.PrivilegedExceptionAction() {
                public Object run() throws Exception {
                            try {
                                file = File.createTempFile("jar_cache", null);
                                fos = new DeployFileOutputStream(file, url, conn,
                                        isPack);
                            } catch (IOException ioe) {
                                Trace.ignoredException(ioe);
                            }
                            return null;
                        }});
        } catch (java.security.PrivilegedActionException e) {
            Trace.ignoredException(e);
        }
    }
    public OutputStream getBody() throws IOException {
        return new BufferedOutputStream(fos);
    }

    public void abort() {
        try {
            java.security.AccessController.doPrivileged(
                    new java.security.PrivilegedExceptionAction() {
                public Object run() throws Exception {
                            try {
                                if (fos != null) {
                                    // abort the cache request
                                    fos.setAbort(true);
                                    fos.close();
                                }
                                if (file != null) {
                                    file.delete();
                                }
                            } catch (IOException ioe) {
                                Trace.ignoredException(ioe);
                            }
                            return null;
                        }
                    });
        } catch (java.security.PrivilegedActionException e) {
            Trace.ignoredException(e);
        }
    }
}

/**
 * An interface that represents a url based response.
 *
 * @since 1.5
 */
class DeployCacheResponse extends java.net.CacheResponse {
    protected InputStream is;
    protected Map headers;

    DeployCacheResponse(InputStream is, Map headers) {
        this.is = is;
        this.headers = headers;
    }

    /**
     * Returns the response body in terms of a InputStream.
     *
     * @return an InputStream from which the response body can
     *         be accessed
     * @throws IOException if an I/O error occurs while
     *         getting the response body
     */
    public InputStream getBody() throws IOException {
        return is;
    }

    /**
     * Returns the response headers in a Map
     *
     * @return An immutable map from response header field names to
     *         lists of field values.
     * @throws IOException if an I/O error occurs
     *            while reading the response body
     */
    public Map getHeaders() throws IOException {
        return headers;
    }

    /**
     * Dump headers.
     */
    private void dumpHeaders(PrintStream ps) {
        // Iterator all headers
        for (Iterator keyIter = headers.keySet().iterator(); keyIter.hasNext(); ) {
            String key = (String) keyIter.next();

            if (key != null) {
                List values = (List) headers.get(key);

                // Iterate all headers
                for (Iterator listIter = values.iterator(); listIter.hasNext(); ) {
                    String value = (String) listIter.next();

                    if (value != null) {
                        ps.println("key=" + key + ", value=" + value);
                    }
                }
            }
        }
    }
}



/**
 * Represents a cache response originally retrieved through secure
 * means, such as TLS.
 *
 * @since 1.5
 */
class DeploySecureCacheResponse extends java.net.SecureCacheResponse {
    protected InputStream is;
    protected Map headers;

    DeploySecureCacheResponse(InputStream is, Map headers) {
        this.is = is;
        this.headers = headers;
    }

    /**
     * Returns the response body in terms of a InputStream.
     *
     * @return an InputStream from which the response body can
     *         be accessed
     * @throws IOException if an I/O error occurs while
     *         getting the response body
     */
    public InputStream getBody() throws IOException {
        return is;
    }

    /**
     * Returns the response headers in a Map
     *
     * @return An immutable map from response header field names to
     *         lists of field values.
     * @throws IOException if an I/O error occurs
     *            while reading the response body
     */
    public Map getHeaders() throws IOException {
        return headers;
    }

    /**
     * Returns the cipher suite in use on the original connection that
     * retrieved the network resource.
     *
     * @return a string representing the cipher suite
     */
    public String getCipherSuite() {
        return null;
    }

    /**
     * Returns the certificate chain that were sent to the server during
     * handshaking of the original connection that retrieved the
     * network resource.  Note: This method is useful only
     * when using certificate-based cipher suites.
     *
     * @return an immutable List of Certificate representing the
     *           certificate chain that was sent to the server. If no
     *           certificate chain was sent, null will be returned.
     * @see getLocalPrincipal()
     */
    public List getLocalCertificateChain() {
        return null;
    }


    /**
     * Returns the server's certificate chain, which was established as
     * part of defining the session in the original connection that
     * retrieved the network resource, from cache.  Note: This method
     * can be used only when using certificate-based cipher suites;
     * using it with non-certificate-based cipher suites, such as
     * Kerberos, will throw an SSLPeerUnverifiedException.
     *
     * @return an immutable List of Certificate representing the server's
     *         certificate chain
     * @see getPeerPrincipal()
     */
    public List getServerCertificateChain() throws SSLPeerUnverifiedException {
        return null;
    }

    /**
     * Returns the server's principal which was established as part of
     * defining the session during the original connection that
     * retrieved the network resource.
     *
     * @return the server's principal. Returns an X500Principal of the
     * end-entity certiticate for X509-based cipher suites, and
     * KerberosPrincipal for Kerberos cipher suites.
     *
     * @throws SSLPeerUnverifiedException if the peer was not verified
     *
     * @see getServerCertificatePath()
     * @see getLocalPrincipal()
     */
    public Principal getPeerPrincipal() throws SSLPeerUnverifiedException {
        return null;
    }


    /**
     * Returns the principal that was sent to the server during
     * handshaking in the original connection that retrieved the
     * network resource.
     *
     * @return the principal sent to the server. Returns an X500Principal
     * of the end-entity certificate for X509-based cipher suites, and
     * KerberosPrincipal for Kerberos cipher suites. If no principal was
     * sent, then null is returned.
     *
     * @see getLocalCertificatePath()
     * @see getPeerPrincipal()
     */
    public Principal getLocalPrincipal() {
        return null;
    }
}

class EmptyInputStream extends InputStream {
    public EmptyInputStream() {
    }

    public int read()
            throws IOException {
        return -1;
    }
}
