/*
 * @(#)HttpUtils.java	1.11 04/06/11
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.net;

import java.io.InputStream;
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.net.URL;
import java.net.URLConnection;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import com.sun.deploy.util.Trace;


// Some simple HTTP utilities used by the jar cache.
public class HttpUtils {
	private static final String CONNECTION_HEADER = "Connection";
	private static final String CONNECTION_KEEP_ALIVE = "Keep-Alive";
	private static final String PROTOCOL_VERSION_1_1 = "HTTP/1.1";


    // Opens a connection, following redirects.  Similar to
    // HttpURLConnection.openConnectionCheckRedirects(), but returns the
    // URLConnection object instead of an InputStream.  This allows the
    // caller to get the response code from the final host.
    public static HttpURLConnection followRedirects(URLConnection c)
        throws IOException {
        boolean redir;
        int redirects = 0;
        InputStream in = null;
        do {
            if (c instanceof HttpURLConnection) {
                ((HttpURLConnection) c).setInstanceFollowRedirects(false);
            }

            // We want to open the input stream before
            // getting headers, because getHeaderField()
            // et al swallow IOExceptions.
            in = c.getInputStream();
            redir = false;

            if (c instanceof HttpURLConnection) {
                HttpURLConnection http = (HttpURLConnection) c;
                int stat = http.getResponseCode();
                if (stat >= 300 && stat <= 305 &&
                    stat != HttpURLConnection.HTTP_NOT_MODIFIED) {
                    URL base = http.getURL();
                    String loc = http.getHeaderField("Location");
                    URL target = null;
                    if (loc != null) {
                        target = new URL(base, loc);
                    }
                    cleanupConnection(http);
                    if (target == null
                        || !base.getProtocol().equals(target.getProtocol())
                        //|| base.getPort() != target.getPort()
                        //|| !hostsEqual(base, target)
                        || redirects >= 5) {
                        throw new SecurityException("illegal URL redirect");
                    }
                    redir = true;
                    c = target.openConnection();
                    redirects++;
                }
            }
        } while (redir);
        if (!(c instanceof HttpURLConnection)) {
            throw new IOException(c.getURL() + " redirected to non-http URL");
        }
        return (HttpURLConnection)c;
    }
    
    public static URL removeQueryStringFromURL(URL u) {
        URL urlNoQuery = u;
        if (urlNoQuery != null) {
            String urlNoQueryString = urlNoQuery.toString();
            int index = urlNoQueryString.lastIndexOf('?');
            if (index != -1) {
                try {
                    urlNoQuery = new URL(urlNoQueryString.substring(0, index));
                } catch (MalformedURLException mue) {
                    // should not happen
                    Trace.ignoredException(mue);
                }
            }
        }
        return urlNoQuery;
    }


    public static void cleanupConnection(URLConnection conn) {
		if(conn == null || !(conn instanceof HttpURLConnection))
			return;

		try {
			HttpURLConnection httpConn = (HttpURLConnection)conn;
			String respHeader = httpConn.getHeaderField(null);
			String connectionHeader = httpConn.getHeaderField(CONNECTION_HEADER);

			// only cleanup if the connection is persistent
            if((connectionHeader != null && connectionHeader.equalsIgnoreCase(CONNECTION_KEEP_ALIVE)) ||
               (respHeader != null && respHeader.startsWith(PROTOCOL_VERSION_1_1) && connectionHeader == null)){
				int respCode = httpConn.getResponseCode();
				if(respCode < HttpURLConnection.HTTP_BAD_REQUEST) {
					InputStream in = httpConn.getInputStream();
				    if(in != null) {
					    byte[] buffer = new byte[8192];
					    while(in.read(buffer) > 0);
					    in.close();
				    }
				}

			}

		}catch(IOException e) {
		}
	}

}


