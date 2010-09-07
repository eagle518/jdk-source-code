/*
 * @(#)PersistenceService.java	1.24 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;

import java.io.InputStream;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.net.URL;
import java.net.MalformedURLException;

/**
 * <code>PersistenceService</code> provides methods for storing data
 * locally on the client system, even for applications that are running
 * in the untrusted execution environment.  The service is somewhat
 * similar to that which the cookie mechanism provides to HTML-based
 * applications.
 * <p>
 * Each entry in the persistence data store is named with a URL.
 * This provides a similar hierarchical structure as a traditional file
 * system.
 * <p>
 * An application is only allowed to access data stored with a URL that
 * is based on its codebase.  For example, given the codebase
 * <code>http://www.mysite.com/apps/App1/</code>, the application
 * would be allowed to access the data at the associated URLs:
 * <ul>
 *    <li><code>http://www.mysite.com/apps/App1/</code>
 *    <li><code>http://www.mysite.com/apps/</code>
 *    <li><code>http://www.mysite.com/</code>
 * </ul>
 * <p>
 * This scheme allows sharing of data between different applications from
 * the same host.  For example, if another application is located at
 * <code>http://www.mysite.com/apps/App2/</code>, then they can share data
 * between them in the <code>http://www.mysite.com/</code> and
 * <code>http://www.mysite.com/apps/</code> directories.
 * <p>
 * A JNLP client should track the amount of storage that a given
 * application uses.  A <code>PersistenceService</code> implementation
 * provides methods to get the current storage usage and limits and to
 * request more storage. Storage is allocated on a per file basis, but a
 * JNLP Client will typically grant or deny the request based on the total
 * storage is use by an application.
 * <p>
 * Data stored using this mechanism is intended to be a local copy of
 * data stored on a remote server.  The individual entries can be
 * tagged as either <i>cached</i>, meaning the server has an up-to-date
 * copy, <i>dirty</i>, meaning the server does not have an up-to-date
 * copy, or <i>temporary</i>, meaning that the file can always be
 * recreated.
 *
 * @since 1.0
 */

public interface PersistenceService {
    
    public static final int CACHED = 0;
    public static final int TEMPORARY = 1;
    public static final int DIRTY = 2;
    
    /**
     * Creates a new persistent storage entry on the client side named with
     * the given URL.
     *
     * @param url       the URL representing the name of the entry in the
     *                  persistent data store.
     *
     * @param maxsize   maximum size of storage that can be written to this
     *                  entry.
     *
     * @return          the maximum size of storage that got granted, in bytes.
     *
     * @throws MalformedURLException if the application is denied access to
     *                               the persistent data store represented by
     *                               the given URL.
     * @throws IOException if an I/O exception occurs, or the entry already exists.
     */
    public long create(URL url, long maxsize)
        throws MalformedURLException, IOException;

    /**
     * Returns a <code>FileContents</code> object representing the contents
     * of this file.
     *
     * @param url     the URL representing the persistent data store entry.
     *
     * @return the file contents as a FileContents.
     *
     * @throws IOException if an I/O error occurs.
     * @throws MalformedURLException if the application is denied access to
     *                               the persistent data store represented by
     *                               the given URL.
     * @throws FileNotFoundException if a persistence store for the given URL
     *                               is not found.
     */
    public FileContents get(URL url)
        throws MalformedURLException, IOException, FileNotFoundException;
	    
    /**
     * Removes the stream associated with the given URL from the
     * client-side date persistence store.
     *
     * @param url     the URL representing the entry to delete from
     *                the persistent data store.
     *
     * @throws MalformedURLException if the application is denied access to
     *                               the persistent data store represented by
     *                               the given URL.
     * @throws IOException if an I/O exception occurs.
     */
    public void delete(URL url)
        throws MalformedURLException, IOException;
    
    /**
     * Returns an array of Strings containing the names of all the
     * entries for a given URL.
     *
     * @param url    the URL representing the root directory to search for
     *               entry names.
     *
     * @return       a <code>String</code> array containing the entries
     *               names.
     * @throws MalformedURLException if the application is denied access to
     *                               the persistent data store represented by
     *                               the given URL.
     * @throws IOException if an I/O exception occurs.
     */
    public String[] getNames(URL url) throws MalformedURLException, IOException;
    
    /**
     * Returns an <code>int</code> corresponding to the current value
     * of the tag for the persistent data store entry associated with the
     * given URL.
     *
     * @param url     the URL representing the persistent data store entry
     *                for which the tag value is requested.
     *
     * @return        an <code>int</code> containing one of the following
     *                tag values:
     *                <ul>
     *                    <li> {@link PersistenceService#CACHED}
     *                    <li> {@link PersistenceService#TEMPORARY}
     *                    <li> {@link PersistenceService#DIRTY}
     *                </ul>
     *
     * @throws MalformedURLException if the application is denied access to
     *                               the persistent data store represented by
     *                               the given URL.
     * @throws IOException if an I/O exception occurs.
     */
    public int getTag(URL url)
        throws MalformedURLException, IOException;
    
    /**
     * Tags the persistent data store entry associated with the given URL
     * with the given tag value.
     *
     * @param url     the URL representing the persistent data store entry
     *                for which to set the tag value.
     *
     * @param tag     the tag value to set.
     *
     * @throws MalformedURLException if the application is denied access to
     *                               the persistent data store represented by
     *                               the given URL.
     * @throws IOException if an I/O exception occurs.
     */
    public void setTag(URL url, int tag)
        throws MalformedURLException, IOException;
    
}

