/*
 * @(#)AuthCache.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www.protocol.http;

import java.io.IOException;
import java.net.URL;
import java.util.Hashtable;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.Enumeration;
import java.util.HashMap;

/**
 * @author Michael McMahon
 * @version 1.2, 12/19/03
 *
 * Interface provided by internal http authentication cache.
 * NB. This API will be replaced in a future release, and should
 * not be made public.
 */

public interface AuthCache {

    /**
     * Put an entry in the cache. pkey is a string specified as follows:
     *
     * A:[B:]C:D:E[:F]   Between 4 and 6 fields separated by ":"
     *		where the fields have the following meaning:
     * A is "s" or "p" for server or proxy authentication respectively
     * B is optional and is "D", "B", or "N" for digest, basic or ntlm auth.
     * C is either "http" or "https"
     * D is the hostname
     * E is the port number
     * F is optional and if present is the realm
     *  
     * Generally, two entries are created for each AuthCacheValue, 
     * one including the realm and one without the realm.
     * Also, for some schemes (digest) multiple entries may be created
     * with the same pkey, but with a different path value in 
     * the AuthCacheValue.
     */
    public void put (String pkey, AuthCacheValue value);

    /**
     * Get an entry from the cache based on pkey as described above, but also
     * using a pathname (skey) and the cache must return an entry 
     * if skey is a sub-path of the AuthCacheValue.path field.
     */
    public AuthCacheValue get (String pkey, String skey);

    /**
     * remove the entry from the cache whose pkey is specified and
     * whose path is equal to entry.path. If entry is null then
     * all entries with the same pkey should be removed.
     */
    public void remove (String pkey, AuthCacheValue entry);
}
