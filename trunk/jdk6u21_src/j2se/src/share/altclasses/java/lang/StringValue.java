/*
 *  @(#)StringValue.java	1.2 10/03/23
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package java.lang;

import java.util.*;

/**
 * A replacement for java.lang.StringValue that profiles usage on startup to 
 * create a cache of the most frequently used String values. This reduces the
 * allocation and copying of String values.
 */

class StringValue {
    private StringValue() { }

    /**
     * Returns a char array that is a copy of the given char array.
     */
    static char[] from(char[] value) {
        // If value is cached, don't make a copy.
        char[] cachedValue = cache.get(value);
        return (cachedValue != null) ? cachedValue : Arrays.copyOf(value, value.length);
    }

    /** Profile and cache Strings as they are constructed to reduce allocation of
     *  value copies.
     */
    private static final boolean stringCacheEnabled;
    private static StringCache cache;


    static {
        stringCacheEnabled = false;
        cache = new StringCache();
    }

    /**
     * Implementation of String profile & cache
     */
    private static class StringProfile {
        int refs;
        char[] value;

        private StringProfile(char[] value) {
            this.value = value;
            this.refs = 1;
        }
    }

    private static class StringCache {
        private static final int MAX_PROFILE_SIZE = 1 << 5;
        private static final int MAX_CACHE_SIZE = 1 << 3;
        private static final int STRING_CACHE_CREATE_THRESHOLD = 1000000;
        private static final float STRING_FREQ_THRESHOLD = 0.125f;
        private static int allocs;
        final ArrayList<StringProfile> profile;
        char[][] cache;
        private boolean profiling;
        private boolean cacheInitialized;

        private StringCache() {
            profile = new ArrayList<StringProfile>();
            profiling = true;
        }

        private StringProfile removeColdestProfile() {
            int i = 0;
            int index = 0;
            int lowest = 1 << 30;
            // Find coldest profile...
            for (StringProfile prof : profile) {
                if (prof.refs < lowest) {
                    lowest = prof.refs;
                    index = i;
                }
                i++;
            }
            // ...and remove it.
            return profile.remove(index);
        }

        private StringProfile removeHottestProfile() {
            int i = 0;
            int index = 0;
            int highest = 0;
            // Find hottest profile...
            for (StringProfile prof : profile) {
                if (prof.refs > highest) {
                    highest = prof.refs;
                    index = i;
                }
                i++;
            }
            // ...and remove it.
            return profile.remove(index);
        }

        private StringProfile getFromProfile(char[] value) {
            // Gets a String value from the profile to increment the ref count
            for (StringProfile prof : profile) {
                if (Arrays.equals(prof.value, value)) {
                    prof.refs++;
                    return prof;
                }
            }
            return null;
        }

        private void putToProfile(char[] value) {
            // Place a String value copy in the profile; remove coldest entry in profile (if required)
            // to keep profile small. Note that a profile entry with a non-zero ref count
            // might be replaced.
            if (profile.size() == MAX_PROFILE_SIZE) {
                removeColdestProfile();
            }
            char[] copy = Arrays.copyOf(value, value.length);
            StringProfile prof = new StringProfile(copy);
            profile.add(prof);
        }

        private void buildCache() {
            // Create list of String values, ordered by "hotness"
            ArrayList<char[]> tmpCache = new ArrayList<char[]>();
            while (tmpCache.size() <= MAX_CACHE_SIZE) {
                StringProfile prof = removeHottestProfile();
                if (prof.refs == 0 || (prof.refs / (float) allocs) < STRING_FREQ_THRESHOLD) {
                    break;
                }
                tmpCache.add(prof.value);
            }
            profile.clear();

            // Create the cache. The cache is an array of String values, sized precisely.
            // The cache will created iff profiling discovered hot String values.
            int size = tmpCache.size();
            if (size > 0) {
                cache = new char[size][];
                int i = 0;
                for (char[] value : tmpCache) {
                    cache[i++] = value;
                }
                cacheInitialized = true;
            }
            profiling = false;
        }

        private char[] getFromCache(char[] value) {
            if (cacheInitialized) {
                for (char[] cachedValue : cache) {
                    if (Arrays.equals(cachedValue, value)) {
                        return cachedValue;
                    }
                }
            }
            return null;
        }

        private void profile(char[] value) {
            // Need to synchronize operations on profile
            synchronized (profile) {
                if (!profiling) {
                    return;
                }

                // Profile incoming String value
                StringProfile prof = getFromProfile(value);
                if (prof == null) {
                    putToProfile(value);
                }

                // Build the cache when # String allocations reaches threshold
                if (++allocs == STRING_CACHE_CREATE_THRESHOLD) {
                    buildCache();
                }
            }
        }

        private char[] get(char[] value) {
            if (stringCacheEnabled) {
                if (!profiling) {
                    return getFromCache(value);
                }
                profile(value);
            }
            return null;
        }
    }
}
