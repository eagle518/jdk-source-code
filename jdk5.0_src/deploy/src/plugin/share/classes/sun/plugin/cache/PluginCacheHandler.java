package sun.plugin.cache;

import java.io.IOException;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.PrintStream;
import java.io.InputStream;
import java.net.CacheRequest;
import java.net.CacheResponse;
import java.net.ResponseCache;
import java.net.URI;
import java.net.URLConnection;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.List;
import java.util.Iterator;
import java.security.AccessController;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.Principal;
import java.security.cert.Certificate;
import javax.net.ssl.SSLPeerUnverifiedException;
import sun.net.www.MessageHeader;

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
public class PluginCacheHandler extends java.net.ResponseCache 
{
    private boolean inCacheHandler = false;

    public static void reset()
    {
	// Set system wide cache handler
	ResponseCache.setDefault(new PluginCacheHandler());
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
    public synchronized CacheResponse get(URI uri, String rqstMethod, Map requestHeaders) throws IOException
    {
	CacheResponse response = null;

	// Avoid unnecessary recursion
	//
	if (inCacheHandler == false)
	{
	    try
	    {
		inCacheHandler = true;

		// Check plug-in cache first
		//
        CachedFile cf = null;
        try {
            cf = FileCache.get(uri.toURL());
        }catch(IOException e) {
            MessageHeader headers = new MessageHeader();
            headers.add(null, "HTTP/1.1 404 Not Found");
			if (uri.getScheme().equals("https")) {
			    response = new PluginSecureCacheResponse(new EmptyInputStream(), headers.getHeaders());
			} else {
			    response = new PluginCacheResponse(new EmptyInputStream(), headers.getHeaders());
		    }

            return response;
        }

		final CachedFile file = cf;
	    
		InputStream is = null;

		if (file != null) 
		{
		    try {
			is = (InputStream) AccessController.doPrivileged(
			     new PrivilegedExceptionAction() {
				public Object run() throws IOException {
				    return new FileInputStream(file);
			     } 
			});
		    } catch (PrivilegedActionException e) {
			//no-op
		    }

		    if (is != null) 
		    {
			// get the header values from cache
			Map cachedHeaders = file.getHeaderFields().getHeaders();

			if (uri.getScheme().equals("https"))
			    response = new PluginSecureCacheResponse(is, cachedHeaders);
			else
			    response = new PluginCacheResponse(is, cachedHeaders);
		    }
    		}
	    }
	    finally
	    {
		inCacheHandler = false;
	    }
	}

	return response;
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
    public synchronized CacheRequest put(URI uri, URLConnection conn)  throws IOException
    {
	// This is currently not hooked up because plug-in cache
	// hasn't taken advantages of HTTP Client API callback.
	//

	return null;
    }
}


/**
 * An interface that represents a url based response.
 *
 * @since 1.5
 */
class PluginCacheResponse extends java.net.CacheResponse
{
    protected InputStream is;
    protected Map headers;

    PluginCacheResponse(InputStream is, Map headers)
    {
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
    public InputStream getBody() throws IOException
    {
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
    public Map getHeaders() throws IOException
    {
	return headers;
    }

    /**
     * Dump headers.
     */
    private void dumpHeaders(PrintStream ps)
    {
	// Iterator all headers
	for (Iterator keyIter = headers.keySet().iterator(); keyIter.hasNext(); )
	{
	    String key = (String) keyIter.next();

	    if (key != null)
	    {
    		List values = (List) headers.get(key);

		// Iterate all headers
		for (Iterator listIter = values.iterator(); listIter.hasNext(); )
		{
		    String value = (String) listIter.next();

		    if (value != null)
		    {
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
class PluginSecureCacheResponse extends java.net.SecureCacheResponse 
{
    protected InputStream is;
    protected Map headers;

    PluginSecureCacheResponse(InputStream is, Map headers)
    {
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
    public InputStream getBody() throws IOException
    {
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
    public Map getHeaders() throws IOException
    {
	return headers;
    }

    /**
     * Returns the cipher suite in use on the original connection that
     * retrieved the network resource.
     *
     * @return a string representing the cipher suite
     */
    public String getCipherSuite()
    {
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
    public List getLocalCertificateChain()
    {
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
    public List getServerCertificateChain() throws SSLPeerUnverifiedException
    {
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
    public Principal getPeerPrincipal() throws SSLPeerUnverifiedException
    {
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
    public Principal getLocalPrincipal()
    {
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
